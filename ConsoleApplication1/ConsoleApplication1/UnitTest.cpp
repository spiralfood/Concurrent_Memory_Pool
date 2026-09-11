#include"ObjectPool.h"
#include"ConcurrentAlloc.h"

void Alloc1() {
	for (size_t i = 0; i < 5; i++)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}

void Alloc2() {
	for (size_t i = 0; i < 5; i++)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}

void TLSTest() {
	std::thread t1(Alloc1);
	std::thread t2(Alloc2);
	t1.join();
	t2.join();
}

void TestConcurrentAlloc1() {
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(7);
	void* p3 = ConcurrentAlloc(1);
	void* p4 = ConcurrentAlloc(2);
	void* p5 = ConcurrentAlloc(3);
}

void TestConcurrentAlloc2() {
	void* p1 = ConcurrentAlloc(6);
	
	ConcurrentFree(p1);
}

	void BigAlloc() {
		void* p1 = ConcurrentAlloc(257 * 1024);
		ConcurrentFree(p1);
		void* p2 = ConcurrentAlloc(129*8 * 1024);
		ConcurrentFree(p2);

	}

	//int main() {

	//	//TestObjectPool();
	//	//TLSTest();
	//	//Alloc2();
	//	//TestConcurrentAlloc1();
	//	//TestConcurrentAlloc2();
	//	BigAlloc();
	//	return 0;
	//}