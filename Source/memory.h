#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

typedef struct Allocator Allocator;

void* allocate(Allocator* allocator, uint64_t bytes);
void copy_memory(void* to, const void* from, uint64_t bytes);
void deallocate(Allocator* allocator, void* memory, uint64_t bytes);
void zero_memory(void* memory, uint64_t bytes);

#endif // MEMORY_H_
