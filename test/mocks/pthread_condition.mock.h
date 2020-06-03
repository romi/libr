#ifndef ROMI_ROVER_BUILD_AND_TEST_PTHREAD_CONDITION_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_PTHREAD_CONDITION_MOCK_H

#include <pthread.h>
#include "fff.h"

DECLARE_FAKE_VALUE_FUNC(int, pthread_cond_init, pthread_cond_t *, const pthread_condattr_t *)
DECLARE_FAKE_VALUE_FUNC(int, pthread_cond_destroy, pthread_cond_t* )
DECLARE_FAKE_VALUE_FUNC(int, pthread_cond_wait, pthread_cond_t*, pthread_mutex_t* )
DECLARE_FAKE_VALUE_FUNC(int, pthread_cond_signal, pthread_cond_t* )

#endif //ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
