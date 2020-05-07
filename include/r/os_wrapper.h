#ifndef ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H
#define ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H

#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// Wrappers for os calls. Used for mocking when testing.

void        free_wrapper(void *data);
int         gettimeofday_wrapper(struct timeval *__restrict __tv, __timezone_ptr_t __tz);
char *      getcwd_wrapper(char *__buf, size_t __size);
struct tm * localtime_r_wrapper(const time_t *__restrict __timer, struct tm *__restrict __tp);
void *      malloc_wrapper(size_t size);
int         stat_wrapper (const char *__restrict __file, struct stat *__restrict __buf);
int         usleep_wrapper (__useconds_t __useconds);

#endif //ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_H
