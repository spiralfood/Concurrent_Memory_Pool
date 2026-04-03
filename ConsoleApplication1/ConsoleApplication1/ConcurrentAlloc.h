#pragma once

#include"ThreadCache.h"
#include"Common.h"

static void* ConcurrentAlloc(size_t size) {
	//通过TLS每个线程无锁的获得自己专属threadCache对象

	if (pTLSThreadCache == nullptr) {
		pTLSThreadCache = new ThreadCache;

	}
	cout << std::this_thread::get_id() << ":"<<pTLSThreadCache<<"\n";
	return pTLSThreadCache->Allocate(size);
}

static void* ConcurrentFree(void*ptr) {

}
