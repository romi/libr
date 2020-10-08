#include "os_wrapper.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, clock_gettime_wrapper, clockid_t, struct timespec *)
DEFINE_FAKE_VALUE_FUNC(struct tm * , localtime_r_wrapper, const time_t *, struct tm *)

DEFINE_FAKE_VALUE_FUNC(int, usleep_wrapper, __useconds_t)
DEFINE_FAKE_VALUE_FUNC(int, open_wrapper, const char *, int)
DEFINE_FAKE_VALUE_FUNC(int, close_wrapper, int)
DEFINE_FAKE_VOID_FUNC(exit_wrapper, int)
