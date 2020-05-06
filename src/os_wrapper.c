#include <stdlib.h>
#include <unistd.h>
#include "os_wrapper.h"

void * malloc_wrapper(size_t size)
{
    return malloc(size);
}

void free_wrapper(void *data)
{
    free(data);
}

int gettimeofday_wrapper(struct timeval *__restrict __tv, __timezone_ptr_t __tz)
{
    return gettimeofday( __tv, __tz);
}

struct tm * localtime_r_wrapper(const time_t *__restrict __timer, struct tm *__restrict __tp)
{
    return localtime_r(__timer, __tp);
}

int usleep_wrapper (__useconds_t __useconds)
{
    return usleep (__useconds);
}