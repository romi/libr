#include "system.mock.h"

DEFINE_FAKE_VALUE_FUNC(ssize_t, read, int, void *, size_t)
DEFINE_FAKE_VALUE_FUNC(ssize_t, write, int, const void *, size_t)
