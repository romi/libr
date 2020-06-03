#include "pthread_mutex.mock.h"

DEFINE_FAKE_VALUE_FUNC(int, pthread_mutex_init, pthread_mutex_t *, const pthread_mutexattr_t *)
DEFINE_FAKE_VALUE_FUNC(int, pthread_mutex_destroy, pthread_mutex_t* )
DEFINE_FAKE_VALUE_FUNC(int, pthread_mutex_lock, pthread_mutex_t* )
DEFINE_FAKE_VALUE_FUNC(int, pthread_mutex_unlock, pthread_mutex_t* )
