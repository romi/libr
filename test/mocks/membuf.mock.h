#ifndef ROMI_ROVER_BUILD_AND_TEST_MEMBUF_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_MEMBUF_MOCK_H

#include "fff.h"
#include "membuf.h"

DECLARE_FAKE_VALUE_FUNC0(membuf_t *, new_membuf)
DECLARE_FAKE_VOID_FUNC(delete_membuf, membuf_t *)

#endif //ROMI_ROVER_BUILD_AND_TEST_MEM_MOCK_H
