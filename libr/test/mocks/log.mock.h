#ifndef ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H

#include "fff.h"
#include "log.h"

DECLARE_FAKE_VOID_FUNC_VARARG(r_debug, const char*, ...)
DECLARE_FAKE_VOID_FUNC_VARARG(r_err, const char*, ...)
DECLARE_FAKE_VOID_FUNC_VARARG(r_warn, const char*, ...)
DECLARE_FAKE_VOID_FUNC_VARARG(r_panic, const char*, ...)

#endif //ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
