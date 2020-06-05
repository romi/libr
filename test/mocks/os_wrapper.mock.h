#ifndef ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_MOCK_H

#include "fff.h"
#include <time.h>
#include <sys/time.h>

DECLARE_FAKE_VOID_FUNC(free_wrapper, void *)
DECLARE_FAKE_VALUE_FUNC(void *, malloc_wrapper, size_t)
DECLARE_FAKE_VALUE_FUNC(void *, memset_wrapper, void *, int, size_t)
DECLARE_FAKE_VALUE_FUNC(void *, realloc_wrapper, void *, size_t)

DECLARE_FAKE_VALUE_FUNC(int, gettimeofday_wrapper, struct timeval *, __timezone_ptr_t)
DECLARE_FAKE_VALUE_FUNC(struct tm * , localtime_r_wrapper, const time_t *, struct tm *)
DECLARE_FAKE_VALUE_FUNC(int, usleep_wrapper, __useconds_t)
DECLARE_FAKE_VOID_FUNC(exit_wrapper, int)

DECLARE_FAKE_VALUE_FUNC(int, open_wrapper, const char *, int)
DECLARE_FAKE_VALUE_FUNC(int, close_wrapper, int)

#endif //ROMI_ROVER_BUILD_AND_TEST_OS_WRAPPER_MOCK_H
