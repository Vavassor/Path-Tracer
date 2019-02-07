#ifndef SOURCE_THREAD_POOL_INTERNAL_H_
#define SOURCE_THREAD_POOL_INTERNAL_H_

#include "atomic.h"
#include "thread_pool.h"

#include <stdbool.h>

typedef struct TaskQueue
{
    Allocator* allocator;
    Task* tasks;
    int cap;
    int count;
    int tail;
} TaskQueue;

typedef struct Thread
{
    ThreadPool* pool;
    uint64_t handle;
    int id;
} Thread;

struct ThreadPool
{
    TaskQueue queue;
    Allocator* allocator;
    Condition* queue_nonempty;
    Condition* task_done;
    Mutex* queue_lock;
    Thread* threads;
    int busy_threads;
    int threads_count;
    bool quit;
};

bool task_queue_create(TaskQueue* queue, Allocator* allocator);
void task_queue_destroy(TaskQueue* queue);
void task_queue_add(TaskQueue* queue, Task task);
bool task_queue_is_empty(TaskQueue* queue);
Task task_queue_remove(TaskQueue* queue);

bool thread_create(Thread* thread);
void thread_join(Thread* thread);
void* thread_start(Thread* thread);

#endif // SOURCE_THREAD_POOL_INTERNAL_H_
