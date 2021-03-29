#include "clock_posix.mock.h"

DEFINE_FAKE_VALUE_FUNC0(uint64_t, clock_timestamp)
DEFINE_FAKE_VALUE_FUNC0(double, clock_time)
DEFINE_FAKE_VALUE_FUNC(char *, clock_datetime_compact, char *, size_t)
