# Concurrent_Memory_Pool
C++ 实现的高并发内存池，基于 Google TCMalloc 核心框架简化开发，实现 ThreadCache/CentralCache/PageCache 三层架构，解决多线程锁竞争和内存碎片问题，性能优于系统 malloc/free。
