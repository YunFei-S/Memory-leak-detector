#include "LeakDetector.h"


int main() {

	// 忘记释放指针 b 申请的内存, 从而导致内存泄露
	int *a = new int;
	char *b = new char[12];

	delete a;

	return 0;

}