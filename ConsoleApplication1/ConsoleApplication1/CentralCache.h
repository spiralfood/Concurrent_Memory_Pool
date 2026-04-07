#pragma once

#include"Common.h"

//µ¥ÀýÄ£Ê½
class CentralCache {
public:
	CentralCache* GetInstance() {
		return &_sInit;
	}
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);
private:
	SpanList _spanLists[NFREE_LIST];
	static CentralCache _sInit;

	CentralCache(){}
	CentralCache(const CentralCache&) = delete;
};