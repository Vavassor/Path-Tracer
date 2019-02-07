#include "atomic.h"

bool atomic_bool_load(AtomicBool* b)
{
    return __atomic_load_n(&b->value, __ATOMIC_SEQ_CST);
}

void atomic_bool_store(AtomicBool* b, bool value)
{
    __atomic_store_n(&b->value, value, __ATOMIC_SEQ_CST);
}


long atomic_int_add(AtomicInt* augend, long addend)
{
    return __atomic_add_fetch(&augend->value, addend, __ATOMIC_SEQ_CST);
}

long atomic_int_load(AtomicInt* i)
{
    return __atomic_load_n(&i->value, __ATOMIC_SEQ_CST);
}

void atomic_int_store(AtomicInt* i, long value)
{
    __atomic_store_n(&i->value, value, __ATOMIC_SEQ_CST);
}

long atomic_int_subtract(AtomicInt* minuend, long subtrahend)
{
    return __atomic_sub_fetch(&minuend->value, subtrahend, __ATOMIC_SEQ_CST);
}
