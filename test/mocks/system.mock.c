#include "system.mock.h"

DEFINE_FAKE_VALUE_FUNC(ssize_t, read, int, void *, size_t)