#include "memory_wrapper.mock.h"

DEFINE_FAKE_VOID_FUNC(free_wrapper, void *)
DEFINE_FAKE_VALUE_FUNC(void *, malloc_wrapper, size_t)
DEFINE_FAKE_VALUE_FUNC(void *, memset_wrapper, void *, int, size_t)
DEFINE_FAKE_VALUE_FUNC(void *, realloc_wrapper, void *, size_t)
