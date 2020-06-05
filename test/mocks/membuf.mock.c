#include "membuf.mock.h"

DEFINE_FAKE_VALUE_FUNC0(membuf_t *, new_membuf)
DEFINE_FAKE_VOID_FUNC(delete_membuf, membuf_t *)
