#include "thread_pool_internal.h"

#include "assert.h"

#if !defined(_WIN32_LEAN_AND_MEAN)
#define _WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

struct Condition
{
    Allocator* allocator;
    CONDITION_VARIABLE handle;
};

struct Mutex
{
    Allocator* allocator;
    CRITICAL_SECTION handle;
};


int get_logical_core_count(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int) info.dwNumberOfProcessors;
}

uint64_t get_thread_id(void)
{
    return (uint64_t) GetCurrentThreadId();
}


Condition* condition_create(Allocator* allocator)
{
    Condition* condition = allocate(allocator, sizeof(Condition));
    if(!condition)
    {
        return NULL;
    }

    condition->allocator = allocator;
    InitializeConditionVariable(&condition->handle);

    return condition;
}

void condition_destroy(Condition* condition)
{
    if(condition)
    {
        deallocate(condition->allocator, condition, sizeof(Condition));
    }
}

void condition_signal_all(Condition* condition)
{
    WakeAllConditionVariable(&condition->handle);
}

void condition_signal_one(Condition* condition)
{
    WakeConditionVariable(&condition->handle);
}

void condition_wait(Condition* condition, Mutex* mutex)
{
    BOOL sleep_result = SleepConditionVariableCS(&condition->handle, &mutex->handle, INFINITE);
    ASSERT(sleep_result);
}


Mutex* mutex_create(Allocator* allocator)
{
    Mutex* mutex = allocate(allocator, sizeof(Mutex));
    if(!mutex)
    {
        return NULL;
    }

    mutex->allocator = allocator;
    InitializeCriticalSection(&mutex->handle);

    return mutex;
}

void mutex_destroy(Mutex* mutex)
{
    if(mutex)
    {
        DeleteCriticalSection(&mutex->handle);

        deallocate(mutex->allocator, mutex, sizeof(Mutex));
    }
}

void mutex_lock(Mutex* mutex)
{
    EnterCriticalSection(&mutex->handle);
}

void mutex_unlock(Mutex* mutex)
{
    LeaveCriticalSection(&mutex->handle);
}


static DWORD WINAPI thread_start_windows(LPVOID parameter)
{
    void* result = thread_start((Thread*) parameter);
    (void) result;

    return 0;
}

bool thread_create(Thread* thread)
{
    HANDLE handle = CreateThread(NULL, 0, thread_start_windows, thread, 0, NULL);

    thread->handle = (uint64_t) handle;

    return handle != INVALID_HANDLE_VALUE;
}

void thread_join(Thread* thread)
{
    DWORD wait_result = WaitForSingleObject((HANDLE) thread->handle, INFINITE);
    ASSERT(wait_result == WAIT_OBJECT_0);
}