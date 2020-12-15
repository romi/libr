#include "pthread.mock.h"

DEFINE_FAKE_VOID_FUNC(test_run_function_mock, void* )

DEFINE_FAKE_VALUE_FUNC(int,
                pthread_create,
                pthread_t *,
                const pthread_attr_t *,
                start_routine_cb,
                void *)

DEFINE_FAKE_VALUE_FUNC(int, pthread_join, pthread_t, void **)
