#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include "memory.h"

typedef struct Condition Condition;
typedef struct Mutex Mutex;

typedef void (*TaskCall)(void* parameter);

typedef struct Task
{
    TaskCall call;
    void* parameter;
} Task;

typedef struct ThreadPool ThreadPool;

int get_logical_core_count(void);
uint64_t get_thread_id(void);

Condition* condition_create(Allocator* allocator);
void condition_destroy(Condition* condition);
void condition_signal_all(Condition* condition);
void condition_signal_one(Condition* condition);
void condition_wait(Condition* condition, Mutex* mutex);

Mutex* mutex_create(Allocator* allocator);
void mutex_destroy(Mutex* mutex);
void mutex_lock(Mutex* mutex);
void mutex_unlock(Mutex* mutex);

void thread_pool_add_task(ThreadPool* pool, Task task);
ThreadPool* thread_pool_create(Allocator* allocator, int threads_count);
void thread_pool_destroy(ThreadPool* pool);
void thread_pool_wait_all(ThreadPool* pool);

#endif // THREAD_POOL_H_
