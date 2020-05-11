#include "os_wrapper.h"

void free_wrapper(void *data)
{
    free(data);
}

void * malloc_wrapper(size_t size)
{
    return malloc(size);
}

void *memcpy_wrapper (void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

void *memset_wrapper (void *source, int value, size_t size)
{
    return memset(source, value, size);
}

void * realloc_wrapper (void *ptr, size_t size)
{
    return realloc(ptr, size);
}

int gettimeofday_wrapper(struct timeval *__restrict __tv, __timezone_ptr_t __tz)
{
    return gettimeofday( __tv, __tz);
}

struct tm * localtime_r_wrapper(const time_t *__restrict __timer, struct tm *__restrict __tp)
{
    return localtime_r(__timer, __tp);
}

char *getcwd_wrapper(char *__buf, size_t __size)
{
    return getcwd(__buf, __size);
}

int stat_wrapper (const char *__restrict __file, struct stat *__restrict __buf)
{
    return stat(__file, __buf);
}

int usleep_wrapper (__useconds_t __useconds)
{
    return usleep (__useconds);
}