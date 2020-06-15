#include "membuf.mock.h"

DEFINE_FAKE_VALUE_FUNC0(membuf_t *, new_membuf)
DEFINE_FAKE_VOID_FUNC(delete_membuf, membuf_t *)
DEFINE_FAKE_VOID_FUNC(membuf_clear, membuf_t *)
DEFINE_FAKE_VOID_FUNC(membuf_put, membuf_t *, char)
DEFINE_FAKE_VOID_FUNC(membuf_append_zero, membuf_t *)
DEFINE_FAKE_VALUE_FUNC(char *, membuf_data, membuf_t *)
DEFINE_FAKE_VALUE_FUNC(int, membuf_len, membuf_t *)
