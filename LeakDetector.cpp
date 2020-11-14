// 这个宏保证 LeakDetector.cpp 中的new 不会被LeakDetector.h中的 宏替换 替换掉
#define NEW_OVERLOAD_IMPLEMENTATION_
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>								//cout 
#include <cstring>								//strlen 和 strcpy
#include "LeakDetector.h"


// 初始化 LeakDetector类中定义的静态变量
size_t LeakDetector::_callCount = 0;


// 我们使用带头节点的双向链表来手动管理内存申请与释放, 头节点的_prev指向最后一个结点, _next指向第一个结点
// 双向链表结构
typedef struct MemoryList {
	struct MemoryList* _prev;
	struct MemoryList* _next;
	size_t _size;								// operator new( )申请的内存大小
	bool   _isArray;							// 是否为申请数组(即使用operator new[]( ) 而不是 operator new( ))
	char*  _file;								// 如果有, 存储存在内存泄漏文件的文件信息
	size_t _line;								// 存储存在内存泄漏位置的行号
} MemoryList;

// 创建一个头结点, 它的前后指针均初始化为指向自己(插入、删除双向链表中结点 和 _LeakDetector( )函数中遍历双向链表时, 这样初始化的作用就体现出来了)。使用静态变量使其只在本文件内有效
// 我们只使用这个头节点的 _prev 和 _next 成员
static MemoryList memoryListHead = { &memoryListHead, &memoryListHead, 0, false, NULL, 0 };


// 保存未释放的内存大小
static size_t memoryAllocated = 0;


// 对双向链表采用头插法分配内存
void* AllocateMemory(size_t size, bool array, char* file, size_t line) {
	// 我们需要为我们管理内存分配的 MemoryList结点 也申请内存
	// 计算新的大小
	size_t newSize = size + sizeof(MemoryList);

	// 把接收到的地址强转为 MemoryList*, 以便我们后续操作
	// 由于重载了new, 所以我们使用 malloc 来申请内存
	MemoryList* newElem = (MemoryList*)malloc(newSize);

	// 更新MemoryList结构成员的值
	newElem->_prev = &memoryListHead;
	newElem->_next = memoryListHead._next;
	newElem->_size = size;						// 注意, 此处为size而不是newSize. 因为我们管理记录的是 new申请的内存, 验证它是否未释放, 存在内存泄漏问题. 申请 newSize的内存(为 MemoryList结点多申请出的内存), 只是为了实现手动管理内存所必须, 这个内存我们一定会释放, 不需关注. 所以保存 时用size而不是newSize
	newElem->_isArray = array;

	// 如果有文件信息, 则保存下来
	if (NULL != file) {
		newElem->_file = (char*)malloc(strlen(file) + 1);
		strcpy(newElem->_file, file);
	}
	else
		newElem->_file = NULL;

	// 保存行号
	newElem->_line = line;

	// 更新双向链表结构
	memoryListHead._next->_prev = newElem;
	memoryListHead._next = newElem;

	// 更新未释放的内存数
	// 我们管理的只是 new申请的内存. 为memoryListHead结点多申请的内存,和为保存文件信息多申请内存无关, 这些内存我们一定会释放, 所以这里只记录size
	memoryAllocated += size;

	// 返回new 申请的内存地址
	// 将newElem强转为char* 类型(保证指针+1时每次加的字节数为1) + memoryListHead所占用字节数( 总共申请的newSize字节数 减去memoryListHead结点占用的字节数, 即为new申请的字节数 )
	return (char*)newElem + sizeof(memoryListHead);
}

// 对双向链表采用头删法手动管理释放内存
// 注意: delete/delete[]时 我们并不知道它操作的是双向链表中的哪一个结点
void  DeleteMemory(void* ptr, bool array) {
	// 注意, 堆的空间自底向上增长. 所以此处为减
	MemoryList* curElem = (MemoryList*)((char*)ptr - sizeof(MemoryList));

	// 如果 new/new[] 和 delete/delete[] 不匹配使用. 直接返回
	if (curElem->_isArray != array)
		return;

	// 更新链表结构
	curElem->_next->_prev = curElem->_prev;
	curElem->_prev->_next = curElem->_next;

	// 更新memoryAllocated值
	memoryAllocated -= curElem->_size;

	// 如果curElem->_file不为NULL, 释放保存文件信息时申请的内存
	if (NULL != curElem->_file)
		free(curElem->_file);

	// 释放内存
	free(curElem);
}


// 重载new/new[]运算符
void* operator new(size_t size, char* file, size_t line){
	return AllocateMemory(size, false, file, line);
}

void* operator new[](size_t size, char* file, size_t line) {
	return AllocateMemory(size, true, file, line);
}

// 重载delete/delete[]运算符
void operator delete(void* ptr) {
	DeleteMemory(ptr, false);
}

void operator delete[](void* ptr) {
	DeleteMemory(ptr, true);
}


// 我们定义的最后一个静态对象析构时调用此函数, 判断是否有内存泄漏, 若有, 则打印出内存泄漏信息
void LeakDetector::_LeakDetector() {
	if (0 == memoryAllocated) {
		std::cout << "恭喜, 您的代码不存在内存泄漏!" << std::endl;
		return;
	}

	// 存在内存泄漏
	// 记录内存泄漏次数
	size_t count = 0;

	// 若不存在内存泄漏, 则双向链表中应该只剩下一个头节点
	// 若存在内存泄漏, 则双向链表中除头节点之外的结点都已泄露，个数即内存泄漏次数
	MemoryList* ptr = memoryListHead._next;
	while ((NULL != ptr) && (&memoryListHead != ptr)) {
		if (true == ptr->_isArray)
			std::cout << "new[] 空间未释放, ";
		else
			std::cout << "new 空间未释放, ";

		std::cout << "指针: " << ptr << " 大小: " << ptr->_size;

		if (NULL != ptr->_file)
			std::cout << " 位于 " << ptr->_file << " 第 " << ptr->_line << " 行";
		else
			std::cout << " (无文件信息)";

		std::cout << std::endl;

		ptr = ptr->_next;
		++count;
	}

	std::cout << "存在" << count << "处内存泄露, 共包括 " << memoryAllocated << " byte." << std::endl;
	return;
}