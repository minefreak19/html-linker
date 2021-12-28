#include "util.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

Cstr shift_arg(int *argc, Cstr *argv[])
{
    assert(argc != NULL);
    assert(argv != NULL);
    assert(*argc > 0);
    assert(*argv != NULL);

    (*argc)--;
    Cstr result = *argv[0];
    (*argv)++;
    return result;
}

void panic_debug(Cstr file, size_t line, Cstr func, Cstr message, ...) 
{
    va_list vargs;
    va_start(vargs, message);

    fprintf(stderr, "%s:%zu: In %s:\n", file, line, func);
    vfprintf(stderr, message, vargs);

    exit(1);
}

void panic_impl(Cstr message, ...)
{
    va_list vargs;
    va_start(vargs, message);

    vfprintf(stderr, message, vargs);

    exit(1);
}


void *notnull_debug(Cstr file, size_t line, Cstr func, Cstr var_name, void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s:%zu: In %s:\n", file, line, func);
        fprintf(stderr, "ERROR: %s is a null pointer\n", var_name);
        exit(1);
    }
    return ptr;
}

void *notnull_impl(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: null pointer. Possibly out of memory.\n");
    }

    return ptr;
}
