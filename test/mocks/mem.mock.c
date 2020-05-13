#include "mem.mock.h"

DEFINE_FAKE_VALUE_FUNC(void *, safe_malloc, size_t, int);
DEFINE_FAKE_VOID_FUNC(safe_free, void *);
DEFINE_FAKE_VALUE_FUNC(void *, safe_realloc, void *, size_t);