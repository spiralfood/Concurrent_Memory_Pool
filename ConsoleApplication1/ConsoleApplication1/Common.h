#pragma once

#include<iostream>
using std::cout;
using std::endl;
#include<vector>
#include<time.h>
#include<assert.h>
#include<mutex>
#include<cmath>
#include<thread>
static const size_t MAX_BYTES = 256 * 1024;
static const size_t NFREE_LIST = 208;

#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;

#endif


static void*& NextObj(void* obj) {
	return *(void**)obj;
}

//管理切分好的小对象的自由链表
class FreeList {
public:
	void Push(void* obj) {
		//头插
		//*(void**)obj = _freeList;
		NextObj(obj)=_freeList;
		_freeList = obj;
	}
	void* pop() {
		assert(_freeList);

		//头删
		void* obj = _freeList;
		_freeList = NextObj(obj);

		return obj;
	}

	bool Empty() {
		return _freeList == nullptr;
	}
	size_t& MaxSize() {
		return _maxSize;
	}
private:
	void* _freeList=nullptr;
	size_t _maxSize = 1;
};

class SizeClass {
public:

	/*size_t _RoundUp(size_t size,size_t alignNum) {
		size_t alignSize;
		if (size%alignNum) {
			alignNum = (size / alignNum + 1) * alignNum;
		}
		else {
			alignSize = size;
		}
		return alignSize;
	}*/

	static inline size_t _RoundUp(size_t bytes,size_t alignNum) {
		
		return (((bytes)+alignNum-1)&~(alignNum-1));
	}
	static inline size_t RoundUp(size_t size) {
		if (size <= 128) {
			return _RoundUp(size, 8);
		}
		else if (size <= 1024) {
			return _RoundUp(size, 16);

		}
		else if (size <= 8 * 1024) {
			return _RoundUp(size, 128);

		}
		else if (size <= 64 * 1024) {
			return _RoundUp(size, 1024);

		}
		else if (size <= 256 * 1024) {
			return _RoundUp(size, 8*1024);

		}
		else {
			assert(false);
			return -1;
		}
	}
	static inline size_t _Index(size_t bytes, size_t align_shift) {
		return ((bytes+((size_t)1<<align_shift)-1)>>align_shift) - 1;
	}

	static inline size_t Index(size_t bytes) {
		assert(bytes <= MAX_BYTES);
		//每个区块有多少链
		static int group_array[4] = { 16,56,56,56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes-128, 3)+group_array[0];
		}
		else if (bytes <= 8*1024) {
			return _Index(bytes-1024, 4)+group_array[0]+group_array[1];
		}
		else if (bytes <= 64*1024) {
			return _Index(bytes-8*1024, 7)+ group_array[0] + group_array[1]+group_array[2];
		}
		else if (bytes <= 256*1024) {
			return _Index(bytes-256*1024, 10)+ group_array[0] + group_array[1] + group_array[2]+group_array[3];
		}
		else {
			assert(false);
		}
		return -1;
	}

	static size_t NumMoveSize(size_t size) {
		assert(size > 0);
		int num = MAX_BYTES / size;
		if (num < 2)num = 2;
		if (num > 512)num = 512;
		return num;
	}

private:

};

//管理多个连续页大块内存跨度结构
class Span {
public:
	PAGE_ID _pageId=0;//大块内存起始页号
	size_t _n=0;//页的数量

	Span* _next=nullptr;//双向链表
	Span* _prev=nullptr;

	size_t _useCount=0;//切好小块内存，被分配给thread cache计数
	void* _freeList=nullptr;//切好的小块内存自由链表 
};

class SpanList {
public:
	SpanList() {
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	void Insert(Span* pos, Span* newSpan) {
		assert(pos);
		assert(newSpan);
		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_prev = prev;
		newSpan->_next = pos;
		pos->_prev = newSpan;

	}

	void Erase(Span* pos) {
		assert(pos);
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span* next= pos->_next;

		prev->_next = next;
		next->_prev = prev;


	}




private:
	Span* _head;
	std::mutex _mtx;//桶锁
};