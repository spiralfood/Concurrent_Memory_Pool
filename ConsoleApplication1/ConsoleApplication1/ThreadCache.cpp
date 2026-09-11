// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include"ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
	//慢开始反馈调节算法
	//1.最开始不会向central cache一次性要太多，要太多可能用不完
	//2.如果不要这个size大小内存需求，batchNum就会不断增长，直到上限
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	assert(batchNum>=1);


	if (_freeLists[index].MaxSize() == batchNum) {
		_freeLists[index].MaxSize() += 1;
	}

	void* start = nullptr;
	void* end = nullptr;

	size_t actualNum=CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum > 0);
	if(actualNum==1){
		assert(start == end);
		return start;
	}
	else {
		_freeLists[index].PushRange(NextObj(start), end,actualNum-1);
		return start;
	}

}

void* ThreadCache::Allocate(size_t size) {
	assert(size <= MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);
	size_t index = SizeClass::Index(size);
	if (!_freeLists[index].Empty()) {
		return _freeLists->pop();
	}
	else {
		return FetchFromCentralCache(index,alignSize);
	}
}

void ThreadCache::Deallocate(void*ptr,size_t size) {
	assert(ptr);
	assert(size <= MAX_BYTES);
	//
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
	//当链表长度大于一次批量申请的内存时就开始释放
	if (_freeLists[index].Size() >= _freeLists[index].MaxSize()) {
		ListTooLong(_freeLists[index], size);
	}

}

void ThreadCache::ListTooLong(FreeList& list, size_t size) {
	void* start = nullptr;
	void* end = nullptr;
	list.PopRange(start, end, list.MaxSize());
	CentralCache::GetInstance()->ReleaseListToSpans(start,size);
}



//
//int main()
//{
//    std::cout << "Hello World!\n";
//}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
