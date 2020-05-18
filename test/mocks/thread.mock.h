#ifndef ROMI_ROVER_BUILD_AND_TEST_THREAD_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_THREAD_MOCK_H
#include "fff.h"
#include "thread.h"

DECLARE_FAKE_VALUE_FUNC0(mutex_t *, new_mutex)
DECLARE_FAKE_VOID_FUNC(delete_mutex, mutex_t* )
DECLARE_FAKE_VOID_FUNC(mutex_lock, mutex_t* )
DECLARE_FAKE_VOID_FUNC(mutex_unlock, mutex_t* )


#endif //ROMI_ROVER_BUILD_AND_TEST_THREAD_MOCK_H
