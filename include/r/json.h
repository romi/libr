/*
  ROMI libr

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  The libr library provides some hardware abstractions and low-level
  utility functions.

  libr is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef _R_JSON_H
#define _R_JSON_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _json_type_t{
        k_json_null = 0,
        k_json_true = 1,
        k_json_false = 2,
        k_json_undefined = 3,

        k_json_object = 100,
        k_json_array = 101,
        k_json_string = 102,
        k_json_number = 103
} json_type_t;

typedef struct _base_t {
        unsigned short refcount;
        json_type_t type;
        union {
                void* data;
                double number;
        } value;
} base_t;

typedef base_t* json_object_t;

#define json_ref(_obj)   json_refcount(_obj, 1)
#define json_unref(_obj) json_refcount(_obj, -1)
void json_refcount(json_object_t object, int32_t val);

/* Return zero to continue, non-zero to stop the iteration. */
typedef int32_t (*json_iterator_t)(const char* key, json_object_t value, void* data);

/* Return zero if all went well, and non-zero when an error
   occured. The error code will be returned by json_serialise(). */
typedef int32_t (*json_writer_t)(void* userdata, const char* s, int32_t len);
//

#define json_type(__o) ((base_t*)__o)->type

//

json_object_t json_load(const char* filename, int* err, char* errmsg, int len);

/*
  object.field
  array[index]
  array[index].field
          
*/
json_object_t json_get(json_object_t obj, const char* expression);
const char* json_getstr(json_object_t obj, const char* expression);
int json_streq(json_object_t obj, const char* expression, const char* value);
double json_getnum(json_object_t obj, const char* expression);


// object

json_object_t json_object_create();
int32_t json_object_length(json_object_t object);
/* Returns the code of the iterator function. */
int32_t json_object_foreach(json_object_t object, json_iterator_t func, void* data);
json_object_t json_object_get(json_object_t object, const char* key);
int32_t json_object_set(json_object_t object, const char* key, json_object_t value);
int32_t json_object_unset(json_object_t object, const char* key);
int json_object_has(json_object_t object, const char* key);

double json_object_getnum(json_object_t object, const char* key);
const char* json_object_getstr(json_object_t object, const char* key);
int json_object_getbool(json_object_t object, const char* key);

int32_t json_object_setnum(json_object_t object, const char* key, double value);
int32_t json_object_setstr(json_object_t object, const char* key, const char* value);
int32_t json_object_setbool(json_object_t object, const char* key, int value);

#define json_isobject(__obj) (__obj->type == k_json_object)

// serialisation

enum {
        k_json_pretty = 1,
        k_json_binary = 2,
};

int32_t json_serialise(json_object_t object, 
                     int32_t flags, 
                     json_writer_t fun, 
                     void* userdata);
int32_t json_tostring(json_object_t object, char* buffer, int32_t buflen);
int32_t json_tostring_pretty(json_object_t object, char* buffer, int32_t buflen);
void json_print(json_object_t object, int32_t flags);

#if !defined(JSON_EMBEDDED)
int32_t json_tofile(json_object_t object, int32_t flags, const char* path);
int32_t json_tofilep(json_object_t object, int32_t flags, FILE* fp);
#endif

// parsing
typedef struct _json_parser_t json_parser_t;

json_parser_t* json_parser_create();
void json_parser_destroy(json_parser_t* parser);

void json_parser_reset(json_parser_t* parser);

int32_t json_parser_errno(json_parser_t* parser);
char* json_parser_errstr(json_parser_t* parser);

int32_t json_parser_feed(json_parser_t* parser, const char* buffer, int32_t len);
int32_t json_parser_done(json_parser_t* parser);

json_object_t json_parser_eval_ext(json_parser_t* parser, const char* s,
                                   int* err, char* errmsg, int len);
json_object_t json_parser_eval(json_parser_t* parser, const char* buffer);

json_object_t json_parser_result(json_parser_t* parser);

json_object_t json_parse_ext(const char* buffer, int* err, char* errmsg, int len);
json_object_t json_parse(const char* buffer);

// null

json_object_t json_null();
#define json_isnull(__obj) ((__obj)->type == k_json_null)

// undefined

json_object_t json_undefined();
#define json_isundefined(__obj) (__obj->type == k_json_undefined)

// boolean

json_object_t json_true();
json_object_t json_false();

#define json_istrue(__obj) (__obj->type == k_json_true)
#define json_isfalse(__obj) (__obj->type == k_json_false)

// number

json_object_t json_number_create(double value);
double json_number_value(json_object_t);

#define json_isnumber(__obj) (__obj->type == k_json_number)

// string

json_object_t json_string_create(const char* s);
const char* json_string_value(json_object_t string);
int32_t json_string_length(json_object_t string);
int32_t json_string_equals(json_object_t string, const char* s);

#define json_isstring(__obj) (__obj->type == k_json_string)

// array

json_object_t json_array_create();
int32_t json_array_length(json_object_t array);
json_object_t json_array_get(json_object_t array, int32_t index);
int32_t json_array_set(json_object_t array, json_object_t value, int32_t index);
int32_t json_array_push(json_object_t array, json_object_t value);

int32_t json_array_gettype(json_object_t object, int32_t index);

double json_array_getnum(json_object_t object, int32_t index);
const char* json_array_getstr(json_object_t object, int32_t index);

int32_t json_array_setnum(json_object_t object, double value, int32_t index);
int32_t json_array_setstr(json_object_t object, const char* value, int32_t index);

#define json_isarray(__obj) (__obj->type == k_json_array)

// json_cleanup frees up the space use by the static variables.
void json_cleanup();

// falsy
#define json_falsy(_x) (json_isnull(_x) || json_isfalse(_x) || json_isundefined(_x) \
                        || (json_isstring(_x) && json_string_length(_x) == 0) \
                        || (json_isnumber(_x) && json_number_value(_x) == 0))

#ifdef __cplusplus
}
#endif

#endif // _R_JSON_H
