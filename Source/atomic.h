#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <stdbool.h>

typedef struct AtomicBool
{
    volatile long value;
} AtomicBool;

typedef struct AtomicInt
{
    volatile long value;
} AtomicInt;

bool atomic_bool_load(AtomicBool* b);
void atomic_bool_store(AtomicBool* b, bool value);

long atomic_int_add(AtomicInt* augend, long addend);
long atomic_int_load(AtomicInt* i);
void atomic_int_store(AtomicInt* i, long value);
long atomic_int_subtract(AtomicInt* minuend, long subtrahend);

#endif // ATOMIC_H_
