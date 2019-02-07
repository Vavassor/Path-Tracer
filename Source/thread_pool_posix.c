#include "thread_pool.h"

#include "atomic.h"
#include "thread_pool_internal.h"

#include <pthread.h>
#include <unistd.h>

#include <assert.h>

#define ASSERT(expression) \
    assert(expression)


struct Condition
{
    Allocator* allocator;
    pthread_cond_t handle;
};

struct Mutex
{
    Allocator* allocator;
    pthread_mutex_t handle;
};


int get_logical_core_count(void)
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

uint64_t get_thread_id(void)
{
    return (uint64_t) pthread_self();
}


Condition* condition_create(Allocator* allocator)
{
    Condition* condition = allocate(allocator, sizeof(Condition));
    if(!condition)
    {
        return NULL;
    }

    condition->allocator = allocator;

    int init_result = pthread_cond_init(&condition->handle, NULL);
    if(init_result != 0)
    {
        deallocate(allocator, condition, sizeof(Condition));
        return NULL;
    }

    return condition;
}

void condition_destroy(Condition* condition)
{
    if(condition)
    {
        int destroy_result = pthread_cond_destroy(&condition->handle);
        ASSERT(destroy_result == 0);

        deallocate(condition->allocator, condition, sizeof(Condition));
    }
}

void condition_signal_all(Condition* condition)
{
    pthread_cond_broadcast(&condition->handle);
}

void condition_signal_one(Condition* condition)
{
    pthread_cond_signal(&condition->handle);
}

void condition_wait(Condition* condition, Mutex* mutex)
{
    int wait_result = pthread_cond_wait(&condition->handle, &mutex->handle);
    ASSERT(wait_result == 0);
}


Mutex* mutex_create(Allocator* allocator)
{
    Mutex* mutex = allocate(allocator, sizeof(Mutex));
    if(!mutex)
    {
        return NULL;
    }

    mutex->allocator = allocator;

    int init_result = pthread_mutex_init(&mutex->handle, NULL);
    if(init_result != 0)
    {
        deallocate(allocator, mutex, sizeof(Mutex));
        return NULL;
    }

    return mutex;
}

void mutex_destroy(Mutex* mutex)
{
    if(mutex)
    {
        int destroy_result = pthread_mutex_destroy(&mutex->handle);
        ASSERT(destroy_result == 0);

        deallocate(mutex->allocator, mutex, sizeof(Mutex));
    }
}

void mutex_lock(Mutex* mutex)
{
    int lock_result = pthread_mutex_lock(&mutex->handle);
    ASSERT(lock_result == 0);
}

void mutex_unlock(Mutex* mutex)
{
    int unlock_result = pthread_mutex_unlock(&mutex->handle);
    ASSERT(unlock_result == 0);
}


static void* thread_start_posix(void* parameter)
{
    return thread_start((Thread*) parameter);
}

bool thread_create(Thread* thread)
{
    pthread_t handle;
    int create_result = pthread_create(&handle, NULL, thread_start_posix, thread);

    thread->handle = (uint64_t) handle;

    return create_result == 0;
}

void thread_join(Thread* thread)
{
    void* result;
    int join_result = pthread_join(thread->handle, &result);
    ASSERT(join_result == 0);
}
