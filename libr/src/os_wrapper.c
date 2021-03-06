#include "r/os_wrapper.h"

// Memory
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

// Time
int clock_gettime_wrapper (clockid_t clk_id, struct timespec *tp)
{
    return clock_gettime( clk_id, tp);
}

struct tm * localtime_r_wrapper(const time_t * __timer, struct tm * __tp)
{
    return localtime_r(__timer, __tp);
}

// Filesystem
char *getcwd_wrapper(char *__buf, size_t __size)
{
    return getcwd(__buf, __size);
}

// OS
int usleep_wrapper (__useconds_t __useconds)
{
    return usleep (__useconds);
}

// FCNTL
int open_wrapper (const char *file_device, int oflag)
{
    return open(file_device, oflag);
}

int close_wrapper (int fd)
{
    return close(fd);
}

void exit_wrapper (int __code)
{
    exit(__code);
}
