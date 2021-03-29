#ifndef ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H

#include "fff.h"
#include "clock_posix.h"

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC0(uint64_t, clock_timestamp)
DECLARE_FAKE_VALUE_FUNC0(double, clock_time)
DECLARE_FAKE_VALUE_FUNC(char *, clock_datetime_compact, char *, size_t)

#ifdef __cplusplus
}
#endif

#endif //ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H
