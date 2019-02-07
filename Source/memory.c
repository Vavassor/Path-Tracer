#include "memory.h"

#include <stdlib.h>

void* allocate(Allocator* allocator, uint64_t bytes)
{
    return calloc(bytes, 1);
}

void copy_memory(void* to, const void* from, uint64_t bytes)
{
    const uint8_t* p0 = from;
    uint8_t* p1 = to;
    if(p0 < p1)
    {
        for(p0 += bytes, p1 += bytes; bytes; bytes -= 1)
        {
            p0 -= 1;
            p1 -= 1;
            *p1 = *p0;
        }
    }
    else
    {
        for(; bytes; bytes -= 1, p0 +=1, p1 += 1)
        {
            *p1 = *p0;
        }
    }
}

void deallocate(Allocator* allocator, void* memory, uint64_t bytes)
{
    return free(memory);
}

void zero_memory(void* memory, uint64_t bytes)
{
    for(uint8_t* p = memory; bytes; bytes -= 1, p += 1)
    {
        *p = 0;
    }
}
