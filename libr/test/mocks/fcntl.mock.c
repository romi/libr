
#include "fcntl.mock.h"

DEFINE_FAKE_VALUE_FUNC_VARARG(int, open, const char *, int, ...)
DEFINE_FAKE_VALUE_FUNC(int, close, int)
