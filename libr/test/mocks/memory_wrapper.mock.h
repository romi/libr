#ifndef ROMI_ROVER_BUILD_AND_TEST_MEMORY_WRAPPER_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_MEMORY_WRAPPER_MOCK_H

#include "fff.h"

DECLARE_FAKE_VOID_FUNC(free_wrapper, void *)
DECLARE_FAKE_VALUE_FUNC(void *, malloc_wrapper, size_t)
DECLARE_FAKE_VALUE_FUNC(void *, memset_wrapper, void *, int, size_t)
DECLARE_FAKE_VALUE_FUNC(void *, realloc_wrapper, void *, size_t)

#endif //ROMI_ROVER_BUILD_AND_TEST_MEMORY_WRAPPER_MOCK_H
