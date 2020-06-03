#ifndef ROMI_ROVER_BUILD_AND_PTHREAD_MOCK_H
#define ROMI_ROVER_BUILD_AND_PTHREAD_MOCK_H

#include <pthread.h>
#include "fff.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

//static int g_pthread_exit_count;
//void *     g_pthread_exit_param;

typedef void *(*start_routine_cb) (void *);

DECLARE_FAKE_VOID_FUNC(test_run_function_mock, void* )

DECLARE_FAKE_VALUE_FUNC(int,
                       pthread_create,
                       pthread_t *,
                       const pthread_attr_t *,
                       start_routine_cb,
                       void *)

DECLARE_FAKE_VALUE_FUNC(int, pthread_join, pthread_t, void **)

//DECLARE_FAKE_VOID_FUNC(pthread_exit, void *)

#ifdef __cplusplus
}
#endif

#endif //ROMI_ROVER_BUILD_AND_TEST_LOG_MOCK_H
