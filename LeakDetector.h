#pragma once



// ע��, ���ǵ�ͷ�ļ���Ҫ�������������Ե�.cpp ��, ����ͷ�ļ��в�Ҫ����"�����"���뼰���ļ�, ����Ӱ�챻���ļ�
#ifndef LEAK_DETECTOR_H_
#define LEAK_DETECTOR_H_
// �и�С����: C/C++���б�׼��ͷ�ļ��궨����������ʽ: _STDIO_H( ��׼�涨�����»�����ǰ׺ )
// ����ƽʱ����Ϊ�˱����Լ�����ĺ���������׼ͷ�ļ�����ĺ귢����ͻ, ����ʹ���»�������׺, ���Ҳ����»�����ǰ׺



// ���ذ汾: operator new/new[]( ), operator delete/delete[]( ) ������
void* operator new(size_t size, char* file, size_t line);
void* operator new[](size_t size, char* file, size_t line);
// ע�⵽, �����������صĺ�����, ��һ�������͵�����������������size_t
// ���е�һ������sizeΪ sizeof�ķ���ֵ, ����Ϊsize_t����
// �����������ĺ���Ϊ �к�, ���������� operator new/new[]( )���Լ��ӵĲ���, �˴�Ҳ������unsigned int. ������� size_t. ԭ����size_t�Ŀ���ֲ�Ժ�. ���ɼ���������
void operator delete(void* ptr);
void operator delete[](void* ptr);


// �������LeakDetector.cpp�ж���, �Է�ֹLeakDetector.cpp��, �����Լ����ص� operator new/new[]( ) �����滻. ��������ڱ������ļ���δ����(���ǳ����ڱ������ļ�����LeakDetector.hͷ�ļ���, ���ı䱻�����ļ��Ĵ���), ���� �滻�������ļ�new�����, ������������ �ļ��� �� �к� ʹ�������Լ������ذ汾operator new/new[]( size_t size, char* file, size_t line )
#ifndef NEW_OVERLOAD_IMPLEMENTATION_
#define new new( __FILE__, __LINE__ )
// Ԥ�����: 
// __FILE__(�����»���): ����ǰԴ�����ļ������ַ�������(������������ô����ڴ�й©�ļ����ļ���)
// __LINE__(�����»���): ����ǰԴ�����ļ��е��кŵ���������(������������ô����ڴ�й©�ļ��ڴ�й©���к�)
#endif


class LeakDetector {
public:
	// LeakDetector.cpp�ͱ����Ե�.cpp����� LeakDetector.hͷ�ļ�
	// �������Դ�ļ��лᴴ��������̬LeakDetector����� exitCounter (������̬�����ͬ��, �������ǵ��������Ծ�Ϊ������(ֻ�ڵ�ǰԴ�ļ���Ч), ��˲����ض���), �����ʱ������������, �����������������, ���������ڴ�й©��⺯��. �����ǵ�Ԥ����ֻ����һ���ڴ�й©��⺯��. ������������һ�������������ľ�̬������ʵ�����ǵ�Ŀ��
	static size_t _callCount;

	LeakDetector() { ++_callCount; }
	~LeakDetector() { if (0 == --_callCount) _LeakDetector(); }

private:
	void _LeakDetector();
};

// ��̬����
static LeakDetector exitCounter;



#endif