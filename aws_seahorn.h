#pragma once

#include <seahorn/seahorn.h>
#include <stdlib.h>

#define AWS_FATAL_PRECONDITION(cond) {assume(cond);}
#define AWS_PRECONDITION(cond) {assume(cond);}
#define AWS_POSTCONDITION(cond) {sassert(cond);}

#define MAX_ITEM_SIZE 2
#define MAX_INITIAL_ITEM_ALLOCATION 10000000//9223372036854775808ULL

#define CBMC_OBJECT_BITS 8
#define MAX_MALLOC (SIZE_MAX >> (CBMC_OBJECT_BITS + 1))

void *bounded_malloc(size_t size) {
    __CPROVER_assume(size <= MAX_MALLOC);
    return malloc(size);
}