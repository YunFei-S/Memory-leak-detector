#include "LeakDetector.h"


int main() {

	// �����ͷ�ָ�� b ������ڴ�, �Ӷ������ڴ�й¶
	int *a = new int;
	char *b = new char[12];

	delete a;

	return 0;

}