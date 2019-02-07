#include "thread_pool_internal.h"

#include <stddef.h>


bool task_queue_create(TaskQueue* queue, Allocator* allocator)
{
    const int cap = 32;

    queue->allocator = allocator;
    queue->cap = cap;
    queue->tasks = allocate(allocator, sizeof(Task) * cap);

    return queue->tasks;
}

void task_queue_destroy(TaskQueue* queue)
{
    deallocate(queue->allocator, queue->tasks, sizeof(Task) * queue->cap);
}

void task_queue_add(TaskQueue* queue, Task task)
{
    int head = (queue->tail + queue->count) % queue->cap;
    queue->tasks[head] = task;
    queue->count += 1;
}

bool task_queue_is_empty(TaskQueue* queue)
{
    return queue->count == 0;
}

Task task_queue_remove(TaskQueue* queue)
{
    Task task = queue->tasks[queue->tail];
    queue->count -= 1;
    queue->tail = (queue->tail + 1) % queue->cap;

    return task;
}


void thread_pool_add_task(ThreadPool* pool, Task task)
{
    mutex_lock(pool->queue_lock);
    task_queue_add(&pool->queue, task);
    mutex_unlock(pool->queue_lock);
    condition_signal_one(pool->queue_nonempty);
}

ThreadPool* thread_pool_create(Allocator* allocator, int threads_count)
{
    ThreadPool* pool = allocate(allocator, sizeof(ThreadPool));

    if(!pool)
    {
        return NULL;
    }

    pool->allocator = allocator;
    pool->threads_count = threads_count;

    pool->threads = allocate(allocator, sizeof(Thread) * threads_count);
    if(!pool->threads)
    {
        thread_pool_destroy(pool);
        return NULL;
    }

    bool queue_created = task_queue_create(&pool->queue, allocator);
    if(!queue_created)
    {
        thread_pool_destroy(pool);
        return NULL;
    }

    pool->queue_lock = mutex_create(allocator);
    if(!pool->queue_lock)
    {
        thread_pool_destroy(pool);
        return NULL;
    }

    pool->queue_nonempty = condition_create(allocator);
    if(!pool->queue_nonempty)
    {
        thread_pool_destroy(pool);
        return NULL;
    }

    pool->task_done = condition_create(allocator);
    if(!pool->task_done)
    {
        thread_pool_destroy(pool);
        return NULL;
    }

    for(int thread_index = 0;
            thread_index < pool->threads_count;
            thread_index += 1)
    {
        Thread* thread = &pool->threads[thread_index];
        thread->pool = pool;
        thread->id = thread_index + 1;

        bool created = thread_create(thread);

        if(!created)
        {
            thread_pool_destroy(pool);
            return NULL;
        }
    }

    return pool;
}

void thread_pool_destroy(ThreadPool* pool)
{
    mutex_lock(pool->queue_lock);
    pool->quit = true;
    condition_signal_all(pool->queue_nonempty);
    mutex_unlock(pool->queue_lock);

    for(int thread_index = 0;
            thread_index < pool->threads_count;
            thread_index += 1)
    {
        thread_join(&pool->threads[thread_index]);
    }

    task_queue_destroy(&pool->queue);
    condition_destroy(pool->queue_nonempty);
    mutex_destroy(pool->queue_lock);

    if(pool->threads)
    {
        deallocate(pool->allocator, pool->threads, sizeof(Thread) * pool->threads_count);
        pool->threads = NULL;
    }

    deallocate(pool->allocator, pool, sizeof(ThreadPool));
}

void thread_pool_wait_all(ThreadPool* pool)
{
    mutex_lock(pool->queue_lock);

    while(pool->busy_threads != 0)
    {
        condition_wait(pool->task_done, pool->queue_lock);
    }

    mutex_unlock(pool->queue_lock);
}


void* thread_start(Thread* thread)
{
    ThreadPool* pool = thread->pool;

    for(;;)
    {
        mutex_lock(pool->queue_lock);

        while(task_queue_is_empty(&pool->queue) && !pool->quit)
        {
            condition_wait(pool->queue_nonempty, pool->queue_lock);
        }

        if(pool->quit)
        {
            mutex_unlock(pool->queue_lock);
            break;
        }

        Task task = task_queue_remove(&pool->queue);
        pool->busy_threads += 1;

        mutex_unlock(pool->queue_lock);

        task.call(task.parameter);

        mutex_lock(pool->queue_lock);

        pool->busy_threads -= 1;
        condition_signal_all(pool->task_done);

        mutex_unlock(pool->queue_lock);
    }

    return NULL;
}
