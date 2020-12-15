#ifndef ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H

#include "fff.h"
#include "mem.h"

DECLARE_FAKE_VALUE_FUNC(void *, safe_malloc, size_t, int)
void *safe_malloc_custom_fake(size_t size, int zero);

DECLARE_FAKE_VOID_FUNC(safe_free, void *)
void safe_free_custom_fake(void *mem);

DECLARE_FAKE_VALUE_FUNC(void *, safe_realloc, void *, size_t)
DECLARE_FAKE_VALUE_FUNC(char *, safe_strdup, const char *)


#endif //ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H
