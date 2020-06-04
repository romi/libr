#include "log.mock.h"

DEFINE_FAKE_VOID_FUNC_VARARG(r_err, const char*, ...)
DEFINE_FAKE_VOID_FUNC_VARARG(r_warn, const char*, ...)
DEFINE_FAKE_VOID_FUNC_VARARG(r_panic, const char*, ...)
