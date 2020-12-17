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
#ifndef _R_JSONXX_H
#define _R_JSONXX_H

#ifdef __cplusplus

#include <exception>
#include <vector>
#include <string>
#include "StringUtils.h"
#include "json.h"
#include "log.h"

class JSONError : public std::exception
{
protected:
        std::string _what;
public:
        JSONError() : std::exception() {}
                
        virtual const char* what() const noexcept override { // TODO
                return _what.c_str(); 
        }
};

class JSONTypeError : public JSONError
{
public:
        JSONTypeError(const char *expected) : JSONError() {
                _what = "Invalid type. Expected JsonCpp type ";
                _what += expected;
        }
};

class JSONKeyError : public JSONError
{
public:
        JSONKeyError(const char *key) : JSONError() {
                _what = "Invalid key: ";
                _what += key;
        }
};

class JSONIndexError : public JSONError
{
public:
        JSONIndexError(int index) : JSONError() {
                _what = "Index out of bounds: ";
                _what += std::to_string(index);
        }
};

class JSONParseError : public JSONError
{
public:
        JSONParseError(const char *msg) : JSONError() {
                _what  = msg;
        }
};

class JsonCpp
{
protected:
        json_object_t _obj;

        static int32_t _tostring(void* userdata, const char* s, int32_t len) {
                std::string *serialised = reinterpret_cast<std::string*>(userdata);
                serialised->append(s, s+len);
                return 0;
        }

public:

        static JsonCpp load(const char *filename) {
                int err;
                char errmsg[128];
                json_object_t obj = json_load(filename, &err, errmsg, 128);
                if (err != 0)
                        throw JSONParseError(errmsg);
                JsonCpp json(obj);
                json_unref(obj);
                return json;
        }

        static JsonCpp parse(const char *s) {
                int err;
                char errmsg[128];
                json_object_t obj = json_parse_ext(s, &err, errmsg, 128);
                if (err != 0)
                        throw JSONParseError(errmsg);
                JsonCpp json(obj);
                json_unref(obj);
                return json;
        }

        static JsonCpp construct(const char *format, ...) {
                std::string parse_string;
                va_list ap;
                va_start(ap, format);
                StringUtils::string_vprintf(parse_string, format, ap);
                va_end(ap);

                JsonCpp obj = JsonCpp::parse(parse_string.c_str());
                return obj;
        }
                
        JsonCpp() {
                _obj = json_null();
        }
                
        JsonCpp(const char *s) {
                int err;
                char errmsg[128];
                _obj = json_parse_ext(s, &err, errmsg, 128);
                if (err != 0)
                        throw JSONParseError(errmsg);
        }
                
        JsonCpp(json_object_t obj) {
                _obj = obj;
                json_ref(_obj);
        }
                
        JsonCpp(JsonCpp &json) {
                _obj = json._obj;
                json_ref(_obj);
        }
                
        virtual ~JsonCpp() {
                json_unref(_obj);
        }
        
        JsonCpp &operator= (const JsonCpp &rhs) {
                json_object_t old = _obj;
                _obj = rhs._obj;
                json_ref(_obj);
                json_unref(old);
                return *this;
        }

        JsonCpp &operator= (json_object_t obj) {
                json_object_t old = _obj;
                _obj = obj;
                json_ref(_obj);
                json_unref(old);
                return *this;
        }
                
        json_object_t ptr() {
                return _obj;
        }
                
        const char *tostring(char *buffer, int len) {
                json_tostring(_obj, buffer, len);
                return buffer;
        }
                
        void tostring(std::string &s) {
                std::string serialised;
                json_serialise(_obj, 0, JsonCpp::_tostring, reinterpret_cast<void*>(&serialised));
                s = serialised;
        }
                
        bool isnull() {
                return json_isnull(_obj);
        }
                
        bool isarray() {
                return json_isarray(_obj);
        }
                
        bool isnumber() {
                return json_isnumber(_obj);
        }
                
        bool isstring() {
                return json_isstring(_obj);
        }
                
        bool has(const char *key) {
                return (json_isobject(_obj) && json_object_has(_obj, key));
        }

        JsonCpp get(const char *key) {
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (!json_object_has(_obj, key))
                        throw JSONKeyError(key);
                JsonCpp retval(json_object_get(_obj, key));
                return retval;
        }

        double num(const char *key) {
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (!json_object_has(_obj, key))
                        throw JSONKeyError(key);
                json_object_t value = json_object_get(_obj, key);
                if (!json_isnumber(value))
                        throw JSONTypeError("number");
                return json_number_value(value);
        }

        double num(const char *key, double default_value) {
                double retval = default_value;
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (json_object_has(_obj, key)) {
                        json_object_t value = json_object_get(_obj, key);
                        if (!json_isnumber(value))
                                throw JSONTypeError("number");
                        retval = json_number_value(value);
                }
                return retval;
        }

        const char *str(const char *key) {
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (!json_object_has(_obj, key))
                        throw JSONKeyError(key);
                json_object_t value = json_object_get(_obj, key);
                if (!json_isstring(value))
                        throw JSONTypeError("string");
                return json_string_value(value);
        }

        JsonCpp array(const char *key) {
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (!json_object_has(_obj, key))
                        throw JSONKeyError(key);
                json_object_t value = json_object_get(_obj, key);
                if (!json_isarray(value))
                        throw JSONTypeError("array");
                JsonCpp json(value);
                return json;
        }

        bool boolean(const char *key) {
                if (!json_isobject(_obj))
                        throw JSONTypeError("object");
                if (!json_object_has(_obj, key))
                        throw JSONKeyError(key);
                json_object_t value = json_object_get(_obj, key);
                if (!json_istrue(value) && !json_isfalse(value))
                        throw JSONTypeError("boolean");
                return json_istrue(value);
        }

        JsonCpp get(int index) {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                if (index < 0 || index >= json_array_length(_obj))
                        throw JSONIndexError(index);
                JsonCpp retval = json_array_get(_obj, index);
                return retval;
        }

        double num(int index) {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                if (index < 0 || index >= json_array_length(_obj))
                        throw JSONIndexError(index);
                json_object_t value = json_array_get(_obj, index);
                if (!json_isnumber(value))
                        throw JSONTypeError("number");
                return json_number_value(value);
        }

        const char *str(int index) {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                if (index < 0 || index >= json_array_length(_obj))
                        throw JSONIndexError(index);
                json_object_t value = json_array_get(_obj, index);
                if (!json_isstring(value))
                        throw JSONTypeError("string");
                return json_string_value(value);
        }

        void setstr(const char *s, int index) {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                json_array_setstr(_obj, s, index);
        }

        JsonCpp array(int index) {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                if (index < 0 || index >= json_array_length(_obj))
                        throw JSONIndexError(index);
                json_object_t value = json_array_get(_obj, index);
                if (!json_isarray(value))
                        throw JSONTypeError("array");
                JsonCpp json(value);
                return json;
        }

        int length() {
                if (!json_isarray(_obj))
                        throw JSONTypeError("array");
                return json_array_length(_obj);
        }

        double num() {
                if (!json_isnumber(_obj))
                        throw JSONTypeError("number");
                return json_number_value(_obj);
        }

        const char *str() {
                if (!json_isstring(_obj))
                        throw JSONTypeError("string");
                return json_string_value(_obj);
        }
                
        int32_t foreach(json_iterator_t func, void* data) {
                return json_object_foreach(_obj, func, data);
        }
};

#endif // __cplusplus
#endif // __R_JSONXX_H
