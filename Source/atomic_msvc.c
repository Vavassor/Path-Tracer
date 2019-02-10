#include "atomic.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <intrin.h>

bool atomic_bool_load(AtomicBool* b)
{
    return _InterlockedExchange((volatile long*) &b->value, 0l);
}

void atomic_bool_store(AtomicBool* b, bool value)
{
    _InterlockedExchange((volatile long*) &b->value, (long) value);
}


long atomic_int_add(AtomicInt* augend, long addend)
{
    long contents = _InterlockedExchangeAdd((volatile long*) &augend->value, addend);
    return contents + addend;
}

long atomic_int_load(AtomicInt* i)
{
    return _InterlockedOr((volatile long*) &i->value, 0l);
}

void atomic_int_store(AtomicInt* i, long value)
{
    _InterlockedExchange((volatile long*) &i->value, value);
}

long atomic_int_subtract(AtomicInt* minuend, long subtrahend)
{
    long contents = _InterlockedExchangeAdd((volatile long*) &minuend->value, -subtrahend);
    return contents - subtrahend;
}