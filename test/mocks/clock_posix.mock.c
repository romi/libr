#include "clock_posix.mock.h"

DEFINE_FAKE_VALUE_FUNC0(uint64_t, clock_timestamp)
