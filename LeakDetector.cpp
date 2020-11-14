// ����걣֤ LeakDetector.cpp �е�new ���ᱻLeakDetector.h�е� ���滻 �滻��
#define NEW_OVERLOAD_IMPLEMENTATION_
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>								//cout 
#include <cstring>								//strlen �� strcpy
#include "LeakDetector.h"


// ��ʼ�� LeakDetector���ж���ľ�̬����
size_t LeakDetector::_callCount = 0;


// ����ʹ�ô�ͷ�ڵ��˫���������ֶ������ڴ��������ͷ�, ͷ�ڵ��_prevָ�����һ�����, _nextָ���һ�����
// ˫������ṹ
typedef struct MemoryList {
	struct MemoryList* _prev;
	struct MemoryList* _next;
	size_t _size;								// operator new( )������ڴ��С
	bool   _isArray;							// �Ƿ�Ϊ��������(��ʹ��operator new[]( ) ������ operator new( ))
	char*  _file;								// �����, �洢�����ڴ�й©�ļ����ļ���Ϣ
	size_t _line;								// �洢�����ڴ�й©λ�õ��к�
} MemoryList;

// ����һ��ͷ���, ����ǰ��ָ�����ʼ��Ϊָ���Լ�(���롢ɾ��˫�������н�� �� _LeakDetector( )�����б���˫������ʱ, ������ʼ�������þ����ֳ�����)��ʹ�þ�̬����ʹ��ֻ�ڱ��ļ�����Ч
// ����ֻʹ�����ͷ�ڵ�� _prev �� _next ��Ա
static MemoryList memoryListHead = { &memoryListHead, &memoryListHead, 0, false, NULL, 0 };


// ����δ�ͷŵ��ڴ��С
static size_t memoryAllocated = 0;


// ��˫���������ͷ�巨�����ڴ�
void* AllocateMemory(size_t size, bool array, char* file, size_t line) {
	// ������ҪΪ���ǹ����ڴ����� MemoryList��� Ҳ�����ڴ�
	// �����µĴ�С
	size_t newSize = size + sizeof(MemoryList);

	// �ѽ��յ��ĵ�ַǿתΪ MemoryList*, �Ա����Ǻ�������
	// ����������new, ��������ʹ�� malloc �������ڴ�
	MemoryList* newElem = (MemoryList*)malloc(newSize);

	// ����MemoryList�ṹ��Ա��ֵ
	newElem->_prev = &memoryListHead;
	newElem->_next = memoryListHead._next;
	newElem->_size = size;						// ע��, �˴�Ϊsize������newSize. ��Ϊ���ǹ����¼���� new������ڴ�, ��֤���Ƿ�δ�ͷ�, �����ڴ�й©����. ���� newSize���ڴ�(Ϊ MemoryList������������ڴ�), ֻ��Ϊ��ʵ���ֶ������ڴ�������, ����ڴ�����һ�����ͷ�, �����ע. ���Ա��� ʱ��size������newSize
	newElem->_isArray = array;

	// ������ļ���Ϣ, �򱣴�����
	if (NULL != file) {
		newElem->_file = (char*)malloc(strlen(file) + 1);
		strcpy(newElem->_file, file);
	}
	else
		newElem->_file = NULL;

	// �����к�
	newElem->_line = line;

	// ����˫������ṹ
	memoryListHead._next->_prev = newElem;
	memoryListHead._next = newElem;

	// ����δ�ͷŵ��ڴ���
	// ���ǹ����ֻ�� new������ڴ�. ΪmemoryListHead����������ڴ�,��Ϊ�����ļ���Ϣ�������ڴ��޹�, ��Щ�ڴ�����һ�����ͷ�, ��������ֻ��¼size
	memoryAllocated += size;

	// ����new ������ڴ��ַ
	// ��newElemǿתΪchar* ����(��ָ֤��+1ʱÿ�μӵ��ֽ���Ϊ1) + memoryListHead��ռ���ֽ���( �ܹ������newSize�ֽ��� ��ȥmemoryListHead���ռ�õ��ֽ���, ��Ϊnew������ֽ��� )
	return (char*)newElem + sizeof(memoryListHead);
}

// ��˫���������ͷɾ���ֶ������ͷ��ڴ�
// ע��: delete/delete[]ʱ ���ǲ���֪������������˫�������е���һ�����
void  DeleteMemory(void* ptr, bool array) {
	// ע��, �ѵĿռ��Ե���������. ���Դ˴�Ϊ��
	MemoryList* curElem = (MemoryList*)((char*)ptr - sizeof(MemoryList));

	// ��� new/new[] �� delete/delete[] ��ƥ��ʹ��. ֱ�ӷ���
	if (curElem->_isArray != array)
		return;

	// ��������ṹ
	curElem->_next->_prev = curElem->_prev;
	curElem->_prev->_next = curElem->_next;

	// ����memoryAllocatedֵ
	memoryAllocated -= curElem->_size;

	// ���curElem->_file��ΪNULL, �ͷű����ļ���Ϣʱ������ڴ�
	if (NULL != curElem->_file)
		free(curElem->_file);

	// �ͷ��ڴ�
	free(curElem);
}


// ����new/new[]�����
void* operator new(size_t size, char* file, size_t line){
	return AllocateMemory(size, false, file, line);
}

void* operator new[](size_t size, char* file, size_t line) {
	return AllocateMemory(size, true, file, line);
}

// ����delete/delete[]�����
void operator delete(void* ptr) {
	DeleteMemory(ptr, false);
}

void operator delete[](void* ptr) {
	DeleteMemory(ptr, true);
}


// ���Ƕ�������һ����̬��������ʱ���ô˺���, �ж��Ƿ����ڴ�й©, ����, ���ӡ���ڴ�й©��Ϣ
void LeakDetector::_LeakDetector() {
	if (0 == memoryAllocated) {
		std::cout << "��ϲ, ���Ĵ��벻�����ڴ�й©!" << std::endl;
		return;
	}

	// �����ڴ�й©
	// ��¼�ڴ�й©����
	size_t count = 0;

	// ���������ڴ�й©, ��˫��������Ӧ��ֻʣ��һ��ͷ�ڵ�
	// �������ڴ�й©, ��˫�������г�ͷ�ڵ�֮��Ľ�㶼��й¶���������ڴ�й©����
	MemoryList* ptr = memoryListHead._next;
	while ((NULL != ptr) && (&memoryListHead != ptr)) {
		if (true == ptr->_isArray)
			std::cout << "new[] �ռ�δ�ͷ�, ";
		else
			std::cout << "new �ռ�δ�ͷ�, ";

		std::cout << "ָ��: " << ptr << " ��С: " << ptr->_size;

		if (NULL != ptr->_file)
			std::cout << " λ�� " << ptr->_file << " �� " << ptr->_line << " ��";
		else
			std::cout << " (���ļ���Ϣ)";

		std::cout << std::endl;

		ptr = ptr->_next;
		++count;
	}

	std::cout << "����" << count << "���ڴ�й¶, ������ " << memoryAllocated << " byte." << std::endl;
	return;
}