
#include "termios.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, tcgetattr, int , struct termios *)
DEFINE_FAKE_VALUE_FUNC(int, cfsetspeed, struct termios *, speed_t)
DEFINE_FAKE_VALUE_FUNC(int, tcflush, int, int)
DEFINE_FAKE_VALUE_FUNC(int, tcsetattr, int, int, const struct termios *)