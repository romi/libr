#include "r_mutex.mock.h"

DEFINE_FAKE_VALUE_FUNC0(mutex_t *, new_mutex)
DEFINE_FAKE_VOID_FUNC(delete_mutex, mutex_t* )
DEFINE_FAKE_VOID_FUNC(mutex_lock, mutex_t* )
DEFINE_FAKE_VOID_FUNC(mutex_unlock, mutex_t* )
