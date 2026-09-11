#include"CentralCache.h"
#include"ThreadCache.h"
#include"PageCache.h"
CentralCache CentralCache::_sInit;

Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	//查看当前有没有空余的span
	Span* it = list.Begin();
	while (it != list.End()) {
		if (it->_freeList != nullptr) {
			return it;

		}
		it = it->_next;
	}
	//先把cental cache桶锁解掉，如果其他线程释放内存对象回来，不会阻塞
	list._mtx.unlock();
	 
	
	//说明没了，去pagecache要
	PageCache::GetInstance()->_pageMtx.lock();
	Span*span=PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	span->_isUse = true;
	span->_objSize = size;
	PageCache::GetInstance()->_pageMtx.unlock();

	//对获取span进行划分，不需要加锁，其他线程拿不到这个span


	//计算span的大块内存的起始地址和大块内存的大小
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	//把大块内存切成自由链表连接起来
	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;
	int i = 0;
	//尾插
	while (start < end) {
		++i;
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;;

	}

	//切好span后，把span挂到桶里，在加锁
	list._mtx.lock();

	return span;
}

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size) {
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index],size);
	assert(span);
	assert(span->_freeList);

	//从span中获取batchNum个对象
	//如果不够，有多少拿多少

	start = span->_freeList;
	end = start;
	size_t i=0;
	size_t actualNum = 1;
	while(i<batchNum-1&&NextObj(end)!=nullptr) {
		end = NextObj(end);
		++i;
		++actualNum;
	}
	_spanLists[index]._mtx.unlock();

	return actualNum;
}

void CentralCache::ReleaseListToSpans(void* start, size_t size) {
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();
	while (start) {
		void* next = NextObj(start);
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		NextObj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;

		//都等于0了
		if (span->_useCount == 0) {
			_spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			_spanLists[index]._mtx.unlock();
			PageCache::GetInstance()->_pageMtx.lock();
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
			PageCache::GetInstance()->_pageMtx.unlock();

		}
		start = next;
	}
	_spanLists[index]._mtx.unlock();
}
