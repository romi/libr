#ifndef ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H
#define ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// Wrappers for os calls. Used for mocking when testing.

void        free_wrapper (void *data);
void *      malloc_wrapper (size_t size);
void *      memcpy_wrapper (void *dest, const void *src, size_t n);
void *      memset_wrapper (void *source, int value, size_t size);
void *      realloc_wrapper (void *__ptr, size_t __size);

int         gettimeofday_wrapper (struct timeval *__restrict __tv, __timezone_ptr_t __tz);
struct tm * localtime_r_wrapper (const time_t *__restrict __timer, struct tm *__restrict __tp);

char *      getcwd_wrapper (char *__buf, size_t __size);
int         usleep_wrapper (__useconds_t __useconds);
void        exit_wrapper (int __code);

#endif //ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H
