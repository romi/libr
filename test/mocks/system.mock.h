#ifndef ROMI_ROVER_BUILD_AND_TEST_SYSTEM_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_SYSTEM_MOCK_H

#include <stdio.h>
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(ssize_t, read, int, void *, size_t)

#endif //ROMI_ROVER_BUILD_AND_TEST_SYSTEM_MOCK_H
