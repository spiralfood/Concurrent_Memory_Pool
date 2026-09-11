#pragma once
#include"Common.h"
#include"CentralCache.h"
#include"ObjectPool.h"

class PageCache {
public:
	static PageCache* GetInstance() {
		return &_sInst;
	}

	Span* MapObjectToSpan(void* obj);
	
	void ReleaseSpanToPageCache(Span* span);

	//获取一个k页的span
	Span* NewSpan(size_t k);
	std::mutex _pageMtx;
private:
	ObjectPool<Span> _spanPool;
	SpanList _spanLists[NPAGES];
	PageCache(){}
	PageCache(const PageCache&) = delete;
	static PageCache _sInst;
	std::unordered_map<PAGE_ID, Span*>_idSpanMap;
	std::unordered_map<PAGE_ID, size_t>_idSizeMap;
};