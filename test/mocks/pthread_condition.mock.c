#include "pthread_condition.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, pthread_cond_init, pthread_cond_t *, const pthread_condattr_t *)
DEFINE_FAKE_VALUE_FUNC(int, pthread_cond_destroy, pthread_cond_t* )
DEFINE_FAKE_VALUE_FUNC(int, pthread_cond_wait, pthread_cond_t*, pthread_mutex_t* )
DEFINE_FAKE_VALUE_FUNC(int, pthread_cond_signal, pthread_cond_t* )
