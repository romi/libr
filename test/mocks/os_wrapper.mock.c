#include "os_wrapper.mock.h"

DEFINE_FAKE_VOID_FUNC(free_wrapper, void *);
DEFINE_FAKE_VALUE_FUNC(void *, malloc_wrapper, size_t);
DEFINE_FAKE_VALUE_FUNC(int, gettimeofday_wrapper, struct timeval *, __timezone_ptr_t);
DEFINE_FAKE_VALUE_FUNC(struct tm * , localtime_r_wrapper, const time_t *, struct tm *);
DEFINE_FAKE_VALUE_FUNC(int, stat_wrapper, const char *, struct stat *);
DEFINE_FAKE_VALUE_FUNC(int, usleep_wrapper, __useconds_t);