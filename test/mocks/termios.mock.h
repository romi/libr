#ifndef ROMI_ROVER_BUILD_AND_TERMIOS_MOCK_H
#define ROMI_ROVER_BUILD_AND_TERMIOS_MOCK_H

#include <termios.h>
#include "fff.h"

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_FAKE_VALUE_FUNC(int, tcgetattr, int , struct termios *)
DECLARE_FAKE_VALUE_FUNC(int, cfsetspeed, struct termios *, speed_t)
DECLARE_FAKE_VALUE_FUNC(int, tcflush, int, int)
DECLARE_FAKE_VALUE_FUNC(int, tcsetattr, int, int, const struct termios *)

#ifdef __cplusplus
}
#endif

#endif //ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
