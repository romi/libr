#include "json.mock.h"

DEFINE_FAKE_VALUE_FUNC(int32_t, json_tostring, json_object_t, char*, int32_t)
DEFINE_FAKE_VALUE_FUNC0(json_parser_t*, json_parser_create)
DEFINE_FAKE_VOID_FUNC(json_parser_destroy, json_parser_t* )
DEFINE_FAKE_VALUE_FUNC(json_object_t, json_parser_eval, json_parser_t*, const char*)
