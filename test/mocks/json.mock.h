#ifndef ROMI_ROVER_BUILD_AND_TEST_JSON_MOCK_H
#define ROMI_ROVER_BUILD_AND_TEST_JSON_MOCK_H

#include "fff.h"
#include "json.h"

DECLARE_FAKE_VALUE_FUNC(int32_t, json_tostring, json_object_t, char*, int32_t)
DECLARE_FAKE_VALUE_FUNC0(json_parser_t*, json_parser_create)
DECLARE_FAKE_VOID_FUNC(json_parser_destroy, json_parser_t* )
DECLARE_FAKE_VALUE_FUNC(json_object_t, json_parser_eval, json_parser_t*, const char*)

#endif //ROMI_ROVER_BUILD_AND_TEST_JSON_MOCK_H
