#ifndef ROMI_ROVER_BUILD_AND_TEST_R_MUTEX_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_MUTEX_MOCK_H

#include <pthread.h>
#include "fff.h"
#include "r_mutex.h"

DECLARE_FAKE_VALUE_FUNC(int, pthread_mutex_init, pthread_mutex_t *, const pthread_mutexattr_t *)
DECLARE_FAKE_VALUE_FUNC(int, pthread_mutex_destroy, pthread_mutex_t* )
DECLARE_FAKE_VALUE_FUNC(int, pthread_mutex_lock, pthread_mutex_t* )
DECLARE_FAKE_VALUE_FUNC(int, pthread_mutex_unlock, pthread_mutex_t* )


#endif //ROMI_ROVER_BUILD_AND_TEST_R_MUTEX_MOCK_H
