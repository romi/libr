#ifndef ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H

#include "fff.h"
#include "mem.h"

DECLARE_FAKE_VALUE_FUNC(void *, safe_malloc, size_t, int);
DECLARE_FAKE_VOID_FUNC(safe_free, void *);
DECLARE_FAKE_VALUE_FUNC(void *, safe_realloc, void *, size_t);

#endif //ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H
