#ifndef ROMI_ROVER_BUILD_AND_FCNTL_MOCK_H
#define ROMI_ROVER_BUILD_AND_FCNTL_MOCK_H

#include <fcntl.h>
#include "fff.h"

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC_VARARG(int, open, const char *, int, ...)
DECLARE_FAKE_VALUE_FUNC(int, close, int)

#ifdef __cplusplus
}
#endif

#endif //ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
