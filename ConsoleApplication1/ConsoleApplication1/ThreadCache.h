#pragma once

#include"Common.h"


class ThreadCache {
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	//从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	FreeList  _freeLists[NFREE_LIST];
	

};

static _declspec(thread) ThreadCache* pTLSThreadCache=nullptr;
