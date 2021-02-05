#ifndef ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H

#include "fff.h"
#include "clock_posix.h"

DECLARE_FAKE_VALUE_FUNC0(uint64_t, clock_timestamp)

#endif //ROMI_ROVER_BUILD_AND_TEST_CLOCK_POSIX_MOCK_H
