#include <stdnoreturn.h>
#include "pthread.mock.h"

DEFINE_FAKE_VOID_FUNC(test_run_function_mock, void* )

DEFINE_FAKE_VALUE_FUNC(int,
                pthread_create,
                pthread_t *,
                const pthread_attr_t *,
                start_routine_cb,
                void *)

DEFINE_FAKE_VALUE_FUNC(int, pthread_join, pthread_t, void **)

//#undef  FFF_GCC_FUNCTION_ATTRIBUTES
//#define FFF_GCC_FUNCTION_ATTRIBUTES __attribute__ ((__noreturn__))
//DEFINE_FAKE_VOID_FUNC(pthread_exit,
//                       void *)
//#undef  FFF_GCC_FUNCTION_ATTRIBUTES


//noreturn void pthread_exit(void *data)
//{
//    g_pthread_exit_param = data;
//    ++g_pthread_exit_count;
//}
