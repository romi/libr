/*
    json.c A small JSON parser.

    Copyright (C) 2013  Sony Computer Science Laboratory Paris
    Author: Peter Hanappe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include "r/mem.h"
#include "r/log.h"
#include "r/json.h"

#define HASHTABLE_MIN_SIZE 7
#define HASHTABLE_MAX_SIZE 13845163

char* json_strdup(const char* s);

#define base_type(_b)      (_b)->type
#define base_get(_b,_type) ((_type*)(_b)->value.data)

static base_t* _null = NULL;
static base_t* _true = NULL;
static base_t* _false = NULL;
static base_t* _undefined = NULL;

#define JSON_NEW(_type)            ((_type*)safe_malloc(sizeof(_type), 1))
#define JSON_NEW_ARRAY(_type, _n)  ((_type*)safe_malloc((size_t)(_n)*sizeof(_type), 1))
#define JSON_FREE(_p)              r_free(_p)

/******************************************************************************/

typedef int (*input_read_t)(void *);

typedef struct _string_input_t {
        const char *s;
        int pos;
} string_input_t;

static int string_input_read(void *ptr)
{
        string_input_t *in = (string_input_t*) ptr;
        return (in->s[in->pos] == '\0')? -1 : in->s[in->pos++];
}

static int file_input_read(void *ptr)
{
        FILE *in = (FILE*) ptr;
        int c = fgetc(in);
        return (c == EOF)? -1 : c;
}


/******************************************************************************/

base_t* base_new(json_type_t type, size_t len)
{
        base_t* base = JSON_NEW(base_t);
        base->refcount = 1;
        base->type = type;
        if (len > 0) {
                base->value.data = JSON_NEW_ARRAY(char, len);
        }
        return base;
}

/******************************************************************************/

json_object_t json_null()
{
        if (_null == NULL) {
                _null = base_new(k_json_null, 0);
                _null->type = k_json_null;
        }

	return _null;
}

json_object_t json_true()
{
        if (_true == NULL) {
                _true = base_new(k_json_true, 0);
                _true->type = k_json_true;
        }

	return _true;
}

json_object_t json_false()
{
        if (_false == NULL) {
                _false = base_new(k_json_false, 0);
                _false->type = k_json_false;
        }

	return _false;
}

json_object_t json_undefined()
{
        if (_undefined == NULL) {
                _undefined = base_new(k_json_undefined, 0);
                _undefined->type = k_json_undefined;
        }

	return _undefined;
}

/******************************************************************************/

/* 31 bit hash function */
// TBD: Think about a length check!
uint32_t json_strhash(const char* key)
{
  const uint8_t *p =  (uint8_t *)key;
  uint32_t h = *p;

  if (h) {
    for (p += 1; *p != '\0'; p++) {
      h = (h << 5) - h + *p;
    }
  }

  return h;
}

char* json_strdup(const char* s)
{
	size_t len = strlen(s) + 1;
	char* t = JSON_NEW_ARRAY(char, len);
	strcpy(t, s);
	return t;
}

/******************************************************************************/

json_object_t json_number_create(double value)
{
        base_t *base;
        base = base_new(k_json_object, 0);
        base->type = k_json_number;
        base->value.number = value;
        return base;
}

static void delete_number(base_t* base __attribute__((unused)))
{
}

double json_number_value(json_object_t obj)
{
        return json_isnumber(obj)? obj->value.number : NAN;
}

/******************************************************************************/

typedef struct _string_t {
	size_t length;
	char s[];
} string_t;

json_object_t json_string_create(const char* s)
{
	base_t *base;
	string_t *string;
       	size_t len = strlen(s);

        base = base_new(k_json_string, sizeof(string_t) + len + 1);
        base->type = k_json_string;

	string = base_get(base, string_t);
	string->length = len;
	memcpy(string->s, s, string->length);
	string->s[string->length] = 0;

	return base;
}

static void delete_string(base_t* base)
{
	string_t *string = base_get(base, string_t);
	JSON_FREE(string);
}

const char* json_string_value(json_object_t obj)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_string)
		return NULL;
	string_t* str = base_get(base, string_t);
	return str->s;
}

size_t json_string_length(json_object_t obj)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_string)
		return 0;
	string_t* str = base_get(base, string_t);
	return str->length;
}

int32_t json_string_compare(json_object_t obj1, const char* s)
{
	if (base_type(obj1) != k_json_string)
		return -1;
	string_t* s1 = base_get(obj1, string_t);
	int res = strcmp(s1->s, s);

    return res;
}

int32_t json_string_equals(json_object_t obj1, const char* s)
{
        return json_string_compare(obj1, s) == 0;
}

/******************************************************************************/

typedef struct _array_t {
    size_t length;
	size_t datalen;
	json_object_t data[];
} array_t;

json_object_t json_array_create()
{
	base_t *base;
	array_t *array;
       	size_t len = 4;
        size_t memlen = sizeof(array_t) + len * sizeof(json_object_t);

        base = base_new(k_json_array, memlen);
        base->type = k_json_array;

	array = base_get(base, array_t);
        for (size_t i = 0; i < len; i++)
                array->data[i] = json_undefined();

	array->length = 0;
	array->datalen = len;

	return base;
}

static void delete_array(base_t* base)
{
	array_t *array = base_get(base, array_t);
        size_t i;

        for (i = 0; i < array->length; i++)
                json_unref(array->data[i]);
	JSON_FREE(array);
}

size_t json_array_length(json_object_t array)
{
	if (array->type != k_json_array)
		return 0;
	array_t* a = (array_t*) array->value.data;
	return a->length;	
}

json_object_t json_array_get(json_object_t obj, size_t index)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_array)
		return json_null();
	array_t *array = base_get(base, array_t);
	if (index < array->length) {
                return array->data[index];
	}
	return json_null();
}

double json_array_getnum(json_object_t obj, size_t index)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_array)
		return NAN;
	array_t *array = base_get(base, array_t);
	if (index < array->length)
		return json_number_value(array->data[index]);
	return NAN;
}


size_t json_array_set(json_object_t obj, json_object_t value, size_t index)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_array)
		return 0;
	if (index > MAX_JSON_ARRAY_SIZE)
	    return 0;

	array_t *array = base_get(base, array_t);
	// LOCK
	if (index >= array->datalen) {
		size_t newlen = 8;
		while (newlen <= index) {
			newlen *= 2;
		}
                
        size_t memlen = sizeof(array_t) + newlen * sizeof(json_object_t);
        array_t *newarray = JSON_NEW_ARRAY(array_t, memlen);
		for (size_t i = 0; i < array->datalen; i++)
			newarray->data[i] = array->data[i];
		for (size_t i = array->datalen; i < newlen; i++)
			newarray->data[i] = json_undefined();

		newarray->datalen = newlen;
		newarray->length = array->length;

        base->value.data = newarray;

        JSON_FREE(array);
        array = newarray;
	}

	json_object_t old = array->data[index];
	array->data[index] = value;  // FIXME: write barrier
	if (index >= array->length) 
		array->length = index + 1;
	
        // UNLOCK

        json_ref(value);
        if (old) json_unref(old);

	return index;
}

size_t json_array_push(json_object_t obj, json_object_t value)
{
	base_t* base = (base_t*) obj;
	if (base_type(base) != k_json_array)
		return 0;
	array_t *array = base_get(base, array_t);
	return json_array_set(obj, value, array->length);	
}

size_t json_array_setnum(json_object_t obj, double value, size_t index)
{
        json_object_t num = json_number_create(value);
        size_t retval = json_array_set(obj, num, index);
        json_unref(num);
        return retval;
}

size_t json_array_setstr(json_object_t obj, const char* value, size_t index)
{
        json_object_t s = json_string_create(value);
        size_t retval = json_array_set(obj, s, index);
        json_unref(s);
        return retval;
}

const char* json_array_getstr(json_object_t array, size_t index)
{
        json_object_t val = json_array_get(array, index);
        if (val->type == k_json_string)
                return json_string_value(val);
        else
                return NULL;
}

/******************************************************************************/

typedef struct _hashnode_t hashnode_t;

struct _hashnode_t {
	char* key;
	json_object_t value;
	hashnode_t *next;
};

static hashnode_t* new_json_hashnode(const char* key, json_object_t* value);
static void delete_json_hashnode(hashnode_t *hash_node);
static void delete_json_hashnodes(hashnode_t *hash_node);


static hashnode_t* new_json_hashnode(const char* key, json_object_t* value)
{
	hashnode_t *hash_node = JSON_NEW(hashnode_t);
	hash_node->key = json_strdup(key);
	hash_node->value = *value;
	json_ref(hash_node->value);
	hash_node->next = NULL;
	
	return hash_node;
}

static void delete_json_hashnode(hashnode_t *hash_node)
{
	json_unref(hash_node->value);
	JSON_FREE(hash_node->key);
	JSON_FREE(hash_node);
}

static void delete_json_hashnodes(hashnode_t *hash_node)
{
	while (hash_node) {
		hashnode_t *next = hash_node->next;
		delete_json_hashnode(hash_node);
		hash_node = next;
	}  
}

/******************************************************************************/
// TBD: size_t?
typedef struct _hashtable_t {
	  //  uint32_t refcount;
        uint32_t size;
        uint32_t num_nodes;
	    hashnode_t **nodes;
} hashtable_t;


static hashtable_t* new_hashtable();
static void delete_hashtable(hashtable_t *hashtable);
static int32_t hashtable_set(hashtable_t *hashtable, const char* key, json_object_t value);
static json_object_t hashtable_get(hashtable_t *hashtable, const char* key);
static int32_t hashtable_unset(hashtable_t *hashtable, const char* key);
static int32_t hashtable_foreach(hashtable_t *hashtable, json_iterator_t func, void* data);
static uint32_t hashtable_size(hashtable_t *hashtable);
static void hashtable_resize(hashtable_t *hashtable);
static hashnode_t** hashtable_lookup_node(hashtable_t *hashtable, const char* key);

static hashtable_t* new_hashtable()
{
	hashtable_t *hashtable = JSON_NEW(hashtable_t);
//	hashtable->refcount = 0;
	hashtable->num_nodes = 0;
	hashtable->size = 0;
	hashtable->nodes = NULL;
	
	return hashtable;
}

static void delete_hashtable(hashtable_t *hashtable)
{
	if (hashtable != NULL) {
                if (hashtable->nodes) {
                        for (uint32_t i = 0; i < hashtable->size; i++) {
                                delete_json_hashnodes(hashtable->nodes[i]);
                        }
                        JSON_FREE(hashtable->nodes);
                }
                JSON_FREE(hashtable);
        }
}

static hashnode_t** hashtable_lookup_node(hashtable_t* hashtable, const char* key)
{
	hashnode_t **node;
	
	if (hashtable->nodes == NULL) {
		hashtable->size = HASHTABLE_MIN_SIZE;
		hashtable->nodes = JSON_NEW_ARRAY(hashnode_t*, hashtable->size);	
		for (uint32_t i = 0; i < hashtable->size; i++)
			hashtable->nodes[i] = NULL;
	}
	
	node = &hashtable->nodes[json_strhash(key) % hashtable->size];
	
	while (*node && (strcmp((*node)->key, key) != 0)) {
		node = &(*node)->next;
	}
	
	return node;
}

static int32_t hashtable_set(hashtable_t *hashtable, const char* key, json_object_t value)
{
  
	if ((key == NULL) || (strlen(key) == 0)) {
		return -1;
	}

	hashnode_t **node = hashtable_lookup_node(hashtable, key);

	if (*node) {
		json_object_t oldvalue = (*node)->value;
		(*node)->value = value;
		json_ref((*node)->value);
		json_unref(oldvalue);

	} else {
		*node = new_json_hashnode(key, &value);
		hashtable->num_nodes++;

		if ((3 * hashtable->size <= hashtable->num_nodes)
		    && (hashtable->size < HASHTABLE_MAX_SIZE)) {
			hashtable_resize(hashtable);
		} 
	}

//        char buffer[256];
//        json_tostring(value, buffer, sizeof(buffer));
        //printf("hash node %s - %s\n", key, buffer);

	return 0;
}

static json_object_t hashtable_get(hashtable_t *hashtable, const char* key)
{
	hashnode_t *node;
	node = *hashtable_lookup_node(hashtable, key);
       	return (node)? node->value : json_undefined();
}

static int32_t hashtable_unset(hashtable_t *hashtable, const char* key)
{
	hashnode_t **node, *dest;
  
	node = hashtable_lookup_node(hashtable, key);
	if (*node) {
		dest = *node;
		(*node) = dest->next;
		delete_json_hashnode(dest);
		hashtable->num_nodes--;
		return 0;
	}
	
	return -1;
}

static int32_t hashtable_foreach(hashtable_t *hashtable, json_iterator_t func, void* data)
{
	hashnode_t *node = NULL;
	
	for (uint32_t i = 0; i < hashtable->size; i++) {
		for (node = hashtable->nodes[i]; node != NULL; node = node->next) {
			int32_t r = (*func)(node->key, node->value, data);
                        if (r != 0) return r;
		}
	}
        return 0;
}

static uint32_t hashtable_size(hashtable_t *hashtable)
{
	return hashtable->num_nodes;
}


static void hashtable_resize(hashtable_t *hashtable)
{
	hashnode_t **new_nodes;
	hashnode_t *node;
	hashnode_t *next;
	uint32_t hash_val;
	uint32_t new_size;
	
	new_size = 3 * hashtable->size + 1;
	new_size = (new_size > HASHTABLE_MAX_SIZE)? HASHTABLE_MAX_SIZE : new_size;
	
	new_nodes = JSON_NEW_ARRAY(hashnode_t*, new_size);
	
	for (uint32_t i = 0; i < hashtable->size; i++) {
		for (node = hashtable->nodes[i]; node; node = next) {
			next = node->next;
			hash_val = json_strhash(node->key) % new_size;      
			node->next = new_nodes[hash_val];
			new_nodes[hash_val] = node;
		}
	}
	
	JSON_FREE(hashtable->nodes);
	hashtable->nodes = new_nodes;
	hashtable->size = new_size;
}

/******************************************************************************/

json_object_t json_object_create()
{
        base_t *base;
        hashtable_t* hashtable;

        base = base_new(k_json_object, 0);
        base->type = k_json_object;
        hashtable = new_hashtable();
        base->value.data = hashtable;
        return base;
}

static void delete_object(base_t* base)
{
	hashtable_t *hashtable = base_get(base, hashtable_t);
        delete_hashtable(hashtable);
}

int32_t json_object_set(json_object_t object, const char* key, json_object_t value)
{
	if (object->type != k_json_object) {
		return -1;
	}
	hashtable_t *hashtable = base_get(object, hashtable_t);
	return hashtable_set(hashtable, key, value);
}

int json_object_has(json_object_t object, const char* key)
{
	if (object->type != k_json_object)
		return 0;
	hashtable_t *hashtable = base_get(object, hashtable_t);
	json_object_t val = hashtable_get(hashtable, key);
        return json_isundefined(val)? 0 : 1;
}

static json_object_t json_object_get_key(json_object_t object, const char *key)
{
	if (object->type != k_json_object) {
		return json_null();
	}
	hashtable_t *hashtable = base_get(object, hashtable_t);
	return hashtable_get(hashtable, key);
}

json_object_t json_object_get(json_object_t object, const char* key)
{
    return json_object_get_key(object, key);
}

double json_object_getnum(json_object_t object, const char* key)
{
        json_object_t val = json_object_get(object, key);
        if (json_isnumber(val)) {
                return json_number_value(val);
        } else {
                return NAN;
        }
}

int json_object_getbool(json_object_t object, const char* key)
{
        json_object_t val = json_object_get(object, key);
        if (json_istrue(val)) {
                return 1;
        } else if (json_isfalse(val)) {
                return 0;
        } else {
                return -1;
        }
}

const char* json_object_getstr(json_object_t object, const char* key)
{
        json_object_t val = json_object_get(object, key);
        if (json_isstring(val)) {
                return json_string_value(val);
        } else {
                return NULL;
        }
}

int32_t json_object_unset(json_object_t object, const char* key)
{
	if (object->type != k_json_object)
		return -1;
	hashtable_t *hashtable = base_get(object, hashtable_t);
	return hashtable_unset(hashtable, key);
}


int32_t json_object_setnum(json_object_t object, const char* key, double value)
{
	json_object_t obj = json_number_create(value);
	int32_t r = json_object_set(object, key, obj);
        json_unref(obj);
	return r;
}

int32_t json_object_setstr(json_object_t object, const char* key, const char* value)
{
        if (value == NULL) return -1;
	json_object_t obj = json_string_create(value);
	int32_t r = json_object_set(object, key, obj);
        json_unref(obj);
	return r;
}

int32_t json_object_setbool(json_object_t object, const char* key, int value)
{
	int32_t r;
        if (value == 0)
                r = json_object_set(object, key, json_false());
        else
                r = json_object_set(object, key, json_true());
	return r;
}

int32_t json_object_foreach(json_object_t object, json_iterator_t func, void* data)
{
	if (object->type != k_json_object)
		return -1;
	hashtable_t *hashtable = base_get(object, hashtable_t);
	return hashtable_foreach(hashtable, func, data);
}

size_t json_object_length(json_object_t object)
{
	if (object->type != k_json_object)
		return 0;
	return hashtable_size(base_get(object, hashtable_t));
}

/******************************************************************************/

static void _delete(base_t *base)
{
        if (base != NULL) {
                switch (base->type) {
                case k_json_number: delete_number(base); break;
                case k_json_string: delete_string(base); break;
                case k_json_array: delete_array(base); break;
                case k_json_object: delete_object(base); break;
                case k_json_null:
                case k_json_true:
                case k_json_false:
                case k_json_undefined:
                default: break;
                }

                JSON_FREE(base);
        }
}

void json_refcount(json_object_t obj, int32_t val)
{
	if ((obj == NULL) || (obj->type < 100)) {
                return;
        }
        obj->refcount += val;
        if (obj->refcount <= 0)
                _delete(obj);
}

/******************************************************************************/

typedef struct _json_serialise_t {
        int pretty;
        int sorted;
        int indent;
} json_serialise_t;

int32_t json_serialise_text(json_serialise_t* serialise, 
                          json_object_t object, 
                          json_writer_t fun, 
                          void* userdata);

typedef struct _json_strbuf_t {
        char* s;
        int32_t len;
        int32_t index;
} json_strbuf_t;

static int32_t json_strwriter(void* userdata, const char* s, size_t len)
{
        json_strbuf_t* strbuf = (json_strbuf_t*) userdata;
        while (len-- > 0) {
                if (strbuf->index >= strbuf->len - 1) {
                        return -1;
                }
                strbuf->s[strbuf->index++] = *s++;
        } 
        return 0;
}

int32_t json_tostring(json_object_t object, int32_t flags, char* buffer, int32_t buflen)
{
        json_strbuf_t strbuf = { buffer, buflen, 0 };
        
        int32_t r = json_serialise(object, flags, json_strwriter, (void*) &strbuf);
        strbuf.s[strbuf.index] = 0;
        return r;
}

static int32_t json_file_writer(void* userdata, const char* s, size_t len)
{
        if (len == 0) return 0;
        FILE* fp = (FILE*) userdata;
	size_t n = fwrite(s, len, 1, fp);
	return (n != 1);
}

int32_t json_tofilep(json_object_t object, int32_t flags, FILE* fp)
{
        int32_t res = json_serialise(object, flags, json_file_writer, (void*) fp);
        fprintf(fp, "\n");
        return res;
}

int32_t json_tofile(json_object_t object, int32_t flags, const char* path)
{
        FILE* fp = fopen(path, "w");
        if (fp == NULL)
                return -1;

        int32_t r = json_serialise(object, flags, json_file_writer, (void*) fp);
        fclose(fp);

        return r;
}

void json_print(json_object_t object, int32_t flags)
{
        json_tofilep(object, flags, stdout);
}

int32_t json_serialise(json_object_t object, 
                       int32_t flags, 
                       json_writer_t fun, 
                       void* userdata)
{
        json_serialise_t serialise;
        serialise.pretty = flags & k_json_pretty;
        serialise.sorted = flags & k_json_sort_keys;
        serialise.indent = 0;

//        const char *old_locale = setlocale(LC_ALL, NULL);
        setlocale(LC_NUMERIC, "C.UTF-8");

        int32_t retval = json_serialise_text(&serialise, object, fun, userdata);

//        setlocale(LC_NUMERIC, old_locale);
        return retval;
}

static int32_t json_write(json_writer_t fun, void* userdata, const char* s)
{
	return (*fun)(userdata, s, strlen(s));
}

int32_t json_number_serialize(json_object_t object, 
                              json_writer_t fun, 
                              void* userdata)
{
        char buf[64];
        if (floor(object->value.number) == object->value.number) 
                snprintf(buf, sizeof(buf), "%.0lf", floor(object->value.number));
        else 
                snprintf(buf, sizeof(buf), "%f", object->value.number);
		
        return json_write(fun, userdata, buf);
}

int32_t json_string_serialize(json_object_t object, 
                              json_writer_t fun, 
                              void* userdata)
{
	int32_t r;
        string_t* string = (string_t*) object->value.data;
        r = json_write(fun, userdata, "\"");
        if (r != 0)
                return r;
        
        r = json_write(fun, userdata, string->s);
        if (r != 0)
                return r;
        
        r = json_write(fun, userdata, "\"");
        
        return r;
}

int32_t json_serialise_array(json_serialise_t* serialise, 
                             json_object_t object, 
                             json_writer_t fun, 
                             void* userdata)
{
	int32_t r = 0;
        array_t* array = (array_t*) object->value.data;
        
        r = json_write(fun, userdata, "[");
        if (r != 0)
                return r;

        for (size_t i = 0; i < array->length; i++) {
                if (array->data[i] == NULL)
                        r = json_write(fun, userdata, "null");
                else
                        r = json_serialise_text(serialise, array->data[i], fun, userdata);
                if (r != 0)
                        return r;
                
                if (i < array->length - 1) {
                        r = json_write(fun, userdata, ", ");
                        if (r != 0)
                                return r;
                }
        }
        
        return json_write(fun, userdata, "]");
}

int32_t json_serialise_indent(json_serialise_t* serialise, 
                              json_writer_t fun, 
                              void* userdata)
{
        int32_t r = 0;
        
        if (serialise->pretty) {
                for (int ii = 0; ii < serialise->indent; ii++) {
                        r = json_write(fun, userdata, " ");
                        if (r != 0)
                                break;
                }
        }
        
        return r;
}

int32_t json_serialise_key_value_pair(json_serialise_t* serialise, 
                                      const char *key, 
                                      json_object_t value,
                                      int last,
                                      json_writer_t fun, 
                                      void* userdata)
{
        int32_t r;
        
        r = json_serialise_indent(serialise, fun, userdata);
        if (r != 0)
                return r;
                        
        r = json_write(fun, userdata, "\"");
        if (r != 0)
                return r;
        
        r = json_write(fun, userdata, key);
        if (r != 0)
                return r;
        
        r = json_write(fun, userdata, "\":");
        if (r != 0)
                return r;
                        
        if (serialise->pretty) {
                r = json_write(fun, userdata, " ");
                if (r != 0)
                        return r;
        }
                        
        r = json_serialise_text(serialise, value, fun, userdata);
        if (r != 0)
                return r;
        
        if (!last) {
                r = json_write(fun, userdata, ",");
                if (r != 0)
                        return r;
        }
                        
        if (serialise->pretty) {
                r = json_write(fun, userdata, "\n");
                if (r != 0)
                        return r;
        }

        return 0;
}

void json_hashtable_keys(hashtable_t* hashtable, char** keys)
{
        hashnode_t *node = NULL;
        uint32_t index = 0;
        
        for (uint32_t i = 0; i < hashtable->size; i++) {
                for (node = hashtable->nodes[i]; node != NULL; node = node->next) {
                        keys[index++] = node->key;
                }
        }
}

static int keycmp(const void* ptr1, const void* ptr2)
{
        const char** key1_handle = (const char**) ptr1;
        const char** key2_handle = (const char**) ptr2;
        int retval = strcmp(*key1_handle, *key2_handle);
        return retval;
}

int32_t json_serialise_hashtable_sorted(json_serialise_t* serialise, 
                                        hashtable_t* hashtable, 
                                        json_writer_t fun, 
                                        void* userdata)
{
        int32_t r = 0;

        if (hashtable->num_nodes > 0) {
                char** keys = JSON_NEW_ARRAY(char*, hashtable->num_nodes);
                json_hashtable_keys(hashtable, keys);

                qsort(keys, hashtable->num_nodes, sizeof(char*), keycmp);
        
                for (uint32_t i = 0; i < hashtable->num_nodes; i++) {

                        int last = (i == hashtable->num_nodes - 1);
                        json_object_t value = hashtable_get(hashtable, keys[i]);

                        r = json_serialise_key_value_pair(serialise, keys[i], value,
                                                          last, fun, userdata);
                        if (r !=0)
                                break;
                }
        
                JSON_FREE(keys);
        }
        return r;
}

int32_t json_serialise_hashtable_unsorted(json_serialise_t* serialise, 
                                          hashtable_t* hashtable, 
                                          json_writer_t fun, 
                                          void* userdata)
{
        int32_t r = 0;
        hashnode_t *node = NULL;
        uint32_t count = 0;
        
        for (uint32_t i = 0; i < hashtable->size; i++) {
                for (node = hashtable->nodes[i]; node != NULL; node = node->next) {

                        int last = (count == hashtable->num_nodes - 1);
                        r = json_serialise_key_value_pair(serialise, node->key,
                                                          node->value, last, fun, 
                                                          userdata);
                        if (r != 0)
                                break;
                        
                        count++;
                }
        }
        return r;
}

int32_t json_serialise_object(json_serialise_t* serialise, 
                              json_object_t object, 
                              json_writer_t fun, 
                              void* userdata)
{
        int32_t r;
        hashtable_t* hashtable = (hashtable_t*) object->value.data;
        
        r = json_write(fun, userdata, "{");
        if (r != 0)
                return r;
        
        if (serialise->pretty) {
                r = json_write(fun, userdata, "\n");
                if (r != 0)
                        return r;
        }
        serialise->indent += 4;

        if (serialise->sorted)
                r = json_serialise_hashtable_sorted(serialise, hashtable, fun, userdata);
        else
                r = json_serialise_hashtable_unsorted(serialise, hashtable, fun, userdata);
        
        if (r != 0)
                return r;
        
        serialise->indent -= 4;
        r = json_serialise_indent(serialise, fun, userdata);
        if (r != 0)
                return r;
        
        return json_write(fun, userdata, "}");
}

int32_t json_serialise_text(json_serialise_t* serialise, 
                            json_object_t object, 
                            json_writer_t fun, 
                            void* userdata)
{
	int32_t r = 0;

	switch (object->type) {

	case k_json_number:
                r = json_number_serialize(object, fun, userdata); 
                break;
		
	case k_json_string: 
                r = json_string_serialize(object, fun, userdata); 
                break;

	case k_json_true:
                r = json_write(fun, userdata, "true");
                break;

	case k_json_false:
                r = json_write(fun, userdata, "false");
                break;

	case k_json_undefined:
                r = json_write(fun, userdata, "undefined");
                break;

	case k_json_null:
		r = json_write(fun, userdata, "null");
                break;

	case k_json_array: 
                r = json_serialise_array(serialise, object, fun, userdata);
                break;
 
	case k_json_object: 
                r = json_serialise_object(serialise, object, fun, userdata);
                break;
                
	default:
	    r_warn("json_serialise_text: unknown type");

	}

	return r;
}

/******************************************************************************/

typedef enum _json_error_t{
	k_end_of_string = -5,
	k_stack_overflow = -4,
	k_out_of_memory = -3, // TBD
	k_token_error = -2,
	k_parsing_error = -1,
	k_continue = 0
} json_error_t;

typedef enum _json_token_t{
	k_no_token = -1,
	k_value,
	k_object_start,
	k_object_end,
	k_array_start,
	k_array_end,
	k_string,
	k_number,
	k_true,
	k_false,
	k_null,
	k_colon,
	k_comma,
	k_end // TBD
} json_token_t;


typedef enum _json_parser_switch_t{
	k_parsing_json,
	k_parsing_string,
	k_parsing_number,
	k_parsing_true,
	k_parsing_false,
	k_parsing_null
} json_parser_switch_t;


typedef enum _json_parser_state_t{
        k_parse_value = 0,
        k_array_value_or_end,
        k_array_comma_or_end,
        k_array_value,
        k_object_key_or_end,
        k_object_colon,
        k_object_value,
        k_object_comma_or_end,
        k_object_key,
        k_value_parsed,
        k_state_error
} json_parser_state_t;  


struct _json_parser_t {
	char* buffer;
	size_t buflen;
	size_t bufindex;
	char backslash;
	char unicode;
	int32_t unihex;
	int32_t numstate;
	int32_t error_code;
	char* error_message;
        int quote;

        int linenum;
        int colnum;
        
	int32_t stack_depth;
	json_object_t value_stack[256];
	int32_t value_stack_top;
	json_parser_state_t state_stack[256];
	int32_t state_stack_top;

	json_parser_switch_t parser_switch;
    json_token_t token;
    int unwind_char;
};

static inline int32_t whitespace(int32_t c)
{
	return ((c == ' ') || (c == '\r') || (c == '\n') || (c == '\t'));
}

json_parser_t* json_parser_create()
{
	json_parser_t* parser = JSON_NEW(json_parser_t);
	json_parser_reset(parser);
	return parser;
}

static void destroy_value_stack(json_parser_t* parser)
{
    for (int32_t i = 0; i <= parser->value_stack_top; i++)
        json_unref(parser->value_stack[i]);
    parser->value_stack_top = -1;
    parser->state_stack_top = 0;
    parser->state_stack[0] = k_parse_value;
}


void json_parser_data_destroy(json_parser_t* parser)
{
    if (parser != NULL) {
        destroy_value_stack(parser);
        json_parser_destroy(parser);
    }
}

void json_parser_destroy(json_parser_t* parser)
{
	if (parser != NULL) {
                JSON_FREE(parser->buffer);
                JSON_FREE(parser->error_message);
                JSON_FREE(parser);
        }
}

// TBD
//int32_t json_parser_errno(json_parser_t* parser)
//{
//	return parser->error_code;
//}

char* json_parser_errstr(json_parser_t* parser)
{
	return parser->error_message;
}

static void json_parser_set_error(json_parser_t* parser, int32_t error, const char* message)
{
	parser->error_code = error;
	parser->error_message = json_strdup(message);
}

static int32_t json_parser_append(json_parser_t* parser, int c)
{
	if (parser->bufindex >= parser->buflen) {
		size_t newlen = 2 * parser->buflen;
		if (newlen == 0)
			newlen = 128;
		char* newbuf = JSON_NEW_ARRAY(char, newlen);
		if (parser->buffer != NULL) {
			memcpy(newbuf, parser->buffer, parser->buflen);
			JSON_FREE(parser->buffer);
		}
		parser->buffer = newbuf;
		parser->buflen = newlen;
	}

	parser->buffer[parser->bufindex++] = (char) (c & 0xff);

	return k_continue;
}

void json_parser_reset(json_parser_t* parser)
{
	parser->state_stack_top = 0;
	parser->state_stack[0] = k_parse_value;
	parser->parser_switch = k_parsing_json;
	parser->value_stack_top = -1;
	parser->stack_depth = 256;
    parser->token = k_no_token;
    parser->unwind_char = -1;
	parser->bufindex = 0;
	parser->unicode = 0;
	parser->unihex = 0;
	parser->backslash = 0;
	parser->error_code = 0;
	parser->linenum = 1;
	parser->colnum = 0;
	if (parser->error_message != NULL) {
		JSON_FREE(parser->error_message);
		parser->error_message = NULL;
	}
}

void json_parser_reset_buffer(json_parser_t* parser)
{
	parser->bufindex = 0;
}

void json_parser_unwind(json_parser_t* parser, int c)
{
        parser->unwind_char = c;
}

static inline const char *token_name(int32_t token)
{
        switch (token) {
        case k_object_start: return "k_object_start";
        case k_object_end: return "k_object_end";
        case k_array_start: return "k_array_start";
        case k_array_end: return "k_array_end";
        case k_string: return "k_string";
        case k_number: return "k_number";
        case k_true: return "k_true";
        case k_false: return "k_false";
        case k_null: return "k_null";
        case k_colon: return "k_colon";
        case k_comma: return "k_comma";
        case k_value: return "k_value";
        default: return "unknown token";
        }
}

static inline const char *state_name(json_parser_state_t state)
{
        switch (state) {
        case k_parse_value: return "k_parse_value";
        case k_array_value_or_end: return "k_array_value_or_end";
        case k_array_comma_or_end: return "k_array_comma_or_end";
        case k_array_value: return "k_array_value";
        case k_value_parsed: return "k_value_parsed";
        case k_object_key_or_end: return "k_object_key_or_end";
        case k_object_colon: return "k_object_colon";
        case k_object_value: return "k_object_value";
        case k_object_comma_or_end: return "k_object_comma_or_end";
        case k_object_key: return "k_object_key";
        case k_state_error: return "k_state_error";
        default: return "unknown state";
        }
}


static inline json_error_t push_state(json_parser_t* parser, json_parser_state_t s)
{
        if (parser->state_stack_top + 1 >= parser->stack_depth) {
                json_parser_set_error(parser, k_stack_overflow, "Stack overflow");
                return k_stack_overflow;
        }
  	parser->state_stack[++parser->state_stack_top] = s;
        return k_continue;
}

static inline json_parser_state_t pop_state(json_parser_t* parser)
{
        if (parser->state_stack_top < 0) {
                json_parser_set_error(parser, k_stack_overflow, "Stack overflow");
                return k_state_error;
        }
  	return parser->state_stack[parser->state_stack_top--];
}

static inline json_parser_state_t peek_state(json_parser_t* parser)
{
  	return parser->state_stack[parser->state_stack_top];
}

static inline void set_state(json_parser_t* parser, json_parser_state_t s)
{
  	parser->state_stack[parser->state_stack_top] = s;
}


static inline int32_t push_value(json_parser_t* parser, json_object_t v)
{
        if (parser->value_stack_top + 1 >= parser->stack_depth) {
                json_parser_set_error(parser, k_stack_overflow, "Stack overflow");
                json_unref(v);
                return k_stack_overflow;
        }
  	    parser->value_stack[++parser->value_stack_top] = v;
        return k_continue;
}

static inline json_object_t pop_value(json_parser_t* parser)
{
        if (parser->state_stack_top < 0) {
                json_parser_set_error(parser, k_stack_overflow, "Stack overflow");
                return json_null();
        }
  	return parser->value_stack[parser->value_stack_top--];
}

static inline json_object_t peek_value(json_parser_t* parser)
{
  	return parser->value_stack[parser->value_stack_top];
}

// STATE               + TOKEN        -> STATE STACK                     & ACTION
// *                   + string       -> -                               & PUSH(new string()); TOKEN(value)  
// *                   + number       -> -                               & PUSH(new number()); TOKEN(value)
// *                   + true         -> -                               & PUSH(new true()); TOKEN(value)
// *                   + false        -> -                               & PUSH(new false()); TOKEN(value)
// *                   + null         -> -                               & PUSH(new null()); TOKEN(value)
// *                   + array_start  -> PUSH(array_value_or_end)        & PUSH(new array())
// array_comma_or_end  + comma        -> SET(array_value)                & -
// array_value_or_end  + value        -> SET(array_comma_or_end)         & v=POP(); a=PEEK(); a.push(v)
// array_value         + value        -> SET(array_comma_or_end)         & v=POP(); a=PEEK(); a.push(v)
// array_comma_or_end  + array_end    -> POP()                           & TOKEN(value)
// array_value_or_end  + array_end    -> POP()                           & TOKEN(value)
// ?                   + object_start -> PUSH(object_key_or_end)         & PUSH(new object())
// object_key_or_end   + value        -> SET(object_colon)               & -
// object_key_or_end   + object_end   -> POP()                           & TOKEN(value)
// object_colon        + colon        -> SET(object_value)               & -
// object_value        + value        -> SET(object_comma_or_end)        & v=POP(), k=POP(), o=PEEK(), o.set(k,v) if k==string
// object_comma_or_end + comma       -> SET(object_key)                 & -
// object_comma_or_end + object_end  -> POP()                           & TOKEN(value)
// object_key          + value        -> SET(object_colon)               & -
// parse_value         + value        -> SET(value_parsed)

static int32_t json_parser_token(json_parser_t* parser, json_token_t token)
{
	json_parser_state_t state = parser->state_stack[parser->state_stack_top];
	double d;
    char *non_num_data = NULL;
	json_object_t obj;
	json_object_t v;
	json_object_t k;
	json_error_t r = k_continue;

        if (0)  {
                printf("[");
                for (int i = 0; i <= parser->state_stack_top; i++) {
                        printf("%s", state_name(parser->state_stack[i]));
                        if (i < parser->state_stack_top) printf(", ");
                }
                printf("] + %s", token_name(token));
                if (token == k_string || token == k_number)
                        printf(" (%s)", parser->buffer);
                printf("\n");
        }
        
        switch (token) {
                        
        case k_array_start:
                r = push_value(parser, json_array_create());
                if (r == k_continue)
                        return push_state(parser, k_array_value_or_end);
                break;

        case k_object_start:
                r = push_value(parser, json_object_create());
                if (r == k_continue)
                        return push_state(parser, k_object_key_or_end);
                break;

        case k_string:
                r = push_value(parser, json_string_create(parser->buffer));
                json_parser_reset_buffer(parser);
                if (r == k_continue)
                        return json_parser_token(parser, k_value);
                break;

        case k_number:
            d = strtod(parser->buffer, &non_num_data);
            json_parser_reset_buffer(parser);
            r = push_value(parser, json_number_create(d));
            if (r == k_continue)
                return json_parser_token(parser, k_value);
            break;

        case k_true:
                r = push_value(parser, json_true());
                if (r == k_continue)
                        return json_parser_token(parser, k_value);
                break;

        case k_false:
                r = push_value(parser, json_false());
                if (r == k_continue)
                        return json_parser_token(parser, k_value);
                break;

        case k_null:
                r = push_value(parser, json_null());
                if (r == k_continue)
                        return json_parser_token(parser, k_value);
                break;
        case k_no_token:
        case k_value:
        case k_object_end:
        case k_array_end:
        case k_colon:
        case k_comma:
        case k_end:
        default:
//               r_debug("json_parser_token - unhandled token %d", token);
        break;
        }

        if (r != k_continue)
                return r;
        
	    switch (state) {
	    case k_parse_value:
                if (token == k_value)
                        set_state(parser, k_value_parsed);
                break;
	    case k_array_value_or_end:
                if (token == k_value) {
                    v = pop_value(parser);
                    obj = peek_value(parser);
                    json_array_push(obj, v);
                    json_unref(v);
                    set_state(parser, k_array_comma_or_end);
                }
                else if (token == k_array_end) {
                    pop_state(parser);
                    r = json_parser_token(parser, k_value);
                }
                else {
                    json_parser_set_error(parser, k_parsing_error,
                                          "Expected a value or ']' while parsing array");
                    r = k_parsing_error;
                }
		break;

	case k_array_comma_or_end:
                if (token == k_comma) {
                    set_state(parser, k_array_value);
                }
                else if (token == k_array_end) {
                    pop_state(parser);
                    r = json_parser_token(parser, k_value);
                }
                else {
                    json_parser_set_error(parser, k_parsing_error,"Expected a ',' or ']' while parsing array");
                    r = k_parsing_error;
                }
		break;

        case k_array_value:
            if (token == k_value)
            {
                v = pop_value(parser);
                obj = peek_value(parser);
                json_array_push(obj, v);
                json_unref(v);
                set_state(parser, k_array_comma_or_end);
            }
            else
            {
                json_parser_set_error(parser, k_parsing_error,"Expected a value while parsing array");
                r = k_parsing_error;
            }
		break;
                
        case k_object_key_or_end:
            if (token == k_value) {
                    k = peek_value(parser);
                    if (!json_isstring(k)) {
                            json_parser_set_error(parser, k_parsing_error,"Expected a string as object key");
                            r = k_parsing_error;
                    } else set_state(parser, k_object_colon);
            }
            else if (token == k_object_end) {
                    pop_state(parser);
                    r = json_parser_token(parser, k_value);
            }
            else{
                    json_parser_set_error(parser, k_parsing_error,"Expected a key string or '}' while parsing object");
                    r = k_parsing_error;
            }
		break;
                
        case k_object_colon:
            if (token == k_colon) {
                    set_state(parser, k_object_value);
            }
            else{
                    json_parser_set_error(parser, k_parsing_error,"Expected a ':' while parsing object");
                    r = k_parsing_error;
            }
            break;
                
        case k_object_value:
            if (token == k_value) {
                v = pop_value(parser);
                k = pop_value(parser);
                obj = peek_value(parser);
                json_object_set(obj, json_string_value(k), v);
                json_unref(k);
                json_unref(v);
                set_state(parser, k_object_comma_or_end);
            }
            else{
                json_parser_set_error(parser, k_parsing_error,"The key-value pair has an invalid value");
                r = k_parsing_error;
            }
		break;
                                
        case k_object_comma_or_end:
            if (token == k_comma) {
                set_state(parser, k_object_key);
            }
            else if (token == k_object_end) {
                pop_state(parser);
                r = json_parser_token(parser, k_value);
            }
            else {
                json_parser_set_error(parser, k_parsing_error,"Expected a comma or '}' while parsing object");
                r = k_parsing_error;
            }
		break;
                
        case k_object_key:
            if (token == k_value) {
                k = peek_value(parser);
                if (!json_isstring(k)) {
                    json_parser_set_error(parser, k_parsing_error,
                                          "Expected a string as object key");
                    r = k_parsing_error;
                } else set_state(parser, k_object_colon);
            }
            else{
                json_parser_set_error(parser, k_parsing_error,"Expected a key string while parsing object");
                r = k_parsing_error;
            }
		break;
        case k_value_parsed:
        break;
        case k_state_error:
        default:
                json_parser_set_error(parser, k_parsing_error, "Parse error");
                r = k_parsing_error;
                break;		
	}

	return r;
}

static int32_t json_parser_unicode(json_parser_t* parser, int c)
{
	int32_t r = k_continue;
	char v = 0;
        
	if (('0' <= c) && (c <= '9')) {
		v = (char)(c - '0');
	} else if (('a' <= c) && (c <= 'f')) {
		v = (char)(10 + c - 'a');
	} else if (('A' <= c) && (c <= 'F')) {
		v = (char)(10 + c - 'A');
	} else {
                json_parser_set_error(parser, k_parsing_error,
                                      "Invalid character in escaped unicode character");
                return k_parsing_error;
        }
        
	switch (parser->unicode) {
	case 1: parser->unihex = (v & 0x0f); 
		break;

	case 2: parser->unihex = (parser->unihex << 4) | (v & 0x0f); 
		break;

	case 3: parser->unihex = (parser->unihex << 4) | (v & 0x0f); 
		break;

	case 4: parser->unihex = (parser->unihex << 4) | (v & 0x0f); 
		if ((0 <= parser->unihex) && (parser->unihex <= 0x007f)) {
			r = json_parser_append(parser, (int) (parser->unihex & 0x007f));
		} else if (parser->unihex <= 0x07ff) {
			uint8_t b1 = (uint8_t)(0xc0 | (parser->unihex & 0x07c0) >> 6);
			uint8_t b2 = (uint8_t)(0x80 | (parser->unihex & 0x003f));
			r = json_parser_append(parser, (int) b1);
			if (r != k_continue) return r;
			r = json_parser_append(parser, (int) b2);
		} else if (parser->unihex <= 0xffff) {
			uint8_t b1 = (uint8_t)(0xe0 | (parser->unihex & 0xf000) >> 12);
			uint8_t b2 = (uint8_t)(0x80 | (parser->unihex & 0x0fc0) >> 6);
			uint8_t b3 = (uint8_t)(0x80 | (parser->unihex & 0x003f));
			r = json_parser_append(parser, (int) b1);
			if (r != k_continue) return r;
			r = json_parser_append(parser, (int) b2);
			if (r != k_continue) return r;
			r = json_parser_append(parser, (int) b3);
		} 
		break;
	    default:
	        r_warn("json_parser_unicode - unknown unicode value");
	}
	return r;
}

static int json_invalid_string_char(int c)
{
        // Control characters must be escaped
        if (c >= 0 && c <= 0x1f)
                return 1;
        return 0;
}

static int32_t json_parser_feed_string(json_parser_t* parser, int c)
{
	int32_t r = k_continue;

	if (parser->unicode > 0) {
		r = json_parser_unicode(parser, c);
		if (parser->unicode++ == 4)
			parser->unicode = 0;

	} else if (parser->backslash) {
		parser->backslash = 0;
		switch (c) {
		case 'n': r = json_parser_append(parser, '\n');
			break; 
		case 'r': r = json_parser_append(parser, '\r');
			break; 
		case 't': r = json_parser_append(parser, '\t');
			break; 
		case 'b': r = json_parser_append(parser, '\b');
			break; 
		case 'f': r = json_parser_append(parser, '\f');
			break; 
		case '/': r = json_parser_append(parser, '/'); 
			break;
		case '\\': r = json_parser_append(parser, '\\'); 
			break;
		case '"': r = json_parser_append(parser, '"'); 
			break;
		case 'u': parser->unicode = 1; 
			break;
		default:
                        json_parser_set_error(parser, k_parsing_error,
                                              "Invalid character after escape");
                        r = k_parsing_error;
			break;
		}

	} else if (c == '\\') {
		parser->backslash = 1;

	} else if (c == parser->quote) {
                r = json_parser_append(parser, 0);
                if (r != k_continue)
                        return r;
                parser->parser_switch = k_parsing_json;
                parser->token = k_string;
                
	} else if (json_invalid_string_char(c)) {
                json_parser_set_error(parser, k_parsing_error,
                                      "Invalid character in string");
                r = k_token_error;
	} else {
		r = json_parser_append(parser, c);
	}

	return r;
}

///////////////////////////////////////////////////////////////////
//
//                  +------------+
//     0     1     2|    4     5 |6               10
//  ()-+---+-+-[0]-++-[.]+[0-9]+-+----------------+--()
//     |   | |     |     +-----+ |                |
//     |   | |     |            [e|E]             |
//     +[-]+ +[1-9]+[0-9]+       |                |
//                 |     |       |  +[+]+         |
//                 +-----+       +--+---+---+[0-9]+ 
//                 3             7  +[-]+ 8 +-----+
//                                                9
//
//-----------------------------------------------------------------
//
//   0 + ''    -> 1
//   0 + '-'   -> 1
//   1 + '0'   -> 2
//   1 + [1-9] -> 3
//   3 + [0-9] -> 3
//   3 + ''    -> 2
//   2 + ''    -> 6
//   2 + '.'   -> 4
//   4 + [0-9] -> 5
//   5 + ''    -> 4
//   5 + ''    -> 6
//   6 + ''    -> 10
//   6 + 'e'   -> 7
//   6 + 'E'   -> 7
//   7 + ''    -> 8
//   7 + '-'   -> 8
//   7 + '+'   -> 8
//   8 + [0-9] -> 9
//   9 + ''    -> 10
//   9 + [0-9] -> 9
//
//-----------------------------------------------------------------
//
//   0_1      + '-'   -> 1
//   1        + '0'   -> 2_6_10
//   1        + [1-9] -> 2_3_6_10
//   0_1      + '0'   -> 2_6_10
//   0_1      + [1-9] -> 2_3_6_10
//   2_6_10   + '.'   -> 4
//   2_6_10   + 'e'   -> 7_8
//   2_3_6_10 + [0-9] -> 2_3_6_10
//   2_3_6_10 + '.'   -> 4
//   2_3_6_10 + 'e'   -> 7_8
//   7_8      + '+'   -> 7
//   7_8      + '-'   -> 7
//   7_8      + [0-9] -> 9_10
//   7        + [0-9] -> 9_10
//   9_10     + [0-9] -> 9_10
//   4        + [0-9] -> 5_6_10
//   5_6_10   + [0-9] -> 5_6_10
//   5_6_10   + 'e'   -> 7_8
//
//-----------------------------------------------------------------
enum { _error = 0, _0_1, _1, _2_6_10, _2_3_6_10, _7_8, _7, _9_10, _4, _5_6_10, _state_last };

enum { _other = 0, _min, _plus, _zero, _d19, _dot, _e, _input_last };

char _numtrans[_state_last][_input_last] = {
//        _other, _min,   _plus,  _zero,     _d19,      _dot,   _e,   
	{ _error, _error, _error, _error,    _error,    _error, _error },   // _error
	{ _error, _1,     _error, _2_6_10,   _2_3_6_10, _error, _error },   // _0_1
	{ _error, _error, _error, _2_6_10,   _2_3_6_10, _error, _error },   // _1
	{ _error, _error, _error, _error,    _error,    _4,     _7_8,  },   // _2_6_10
	{ _error, _error, _error, _2_3_6_10, _2_3_6_10, _4,     _7_8,  },   // _2_3_6_10
	{ _error, _7,     _7,     _9_10,     _9_10,     _error, _error },   // _7_8
	{ _error, _error, _error, _9_10,     _9_10,     _error, _error },   // _7
	{ _error, _error, _error, _9_10,     _9_10,     _error, _error },   // _9_10
	{ _error, _error, _error, _5_6_10,   _5_6_10,   _error, _error },   // _4
	{ _error, _error, _error, _5_6_10,   _5_6_10,   _error, _7_8,  }};  // _5_6_10

char _endstate[_state_last] = { 0, 0, 0, 1, 1, 0, 0, 1, 0, 1 };

static inline int32_t json_numinput(json_parser_t* parser, int c)
{
        (void) parser;
	if (('1' <= c) && (c <= '9')) return _d19;
	if (c == '0') return _zero;
	if (c == '.') return _dot;
	if (c == '-') return _min;
	if (c == '+') return _plus;
	if (c == 'e') return _e;
	if (c == 'E') return _e;
	return _other;
}

static int32_t json_parser_feed_number(json_parser_t* parser, int c)
{
	int32_t r = k_token_error;
	
	int32_t input = json_numinput(parser, c);

	int32_t prevstate = parser->numstate;

	parser->numstate = _numtrans[prevstate][input];

	if (parser->numstate != _error) {
		r = json_parser_append(parser, c);

	} else if (_endstate[prevstate]) {
                r = json_parser_append(parser, 0);
			if (r != k_continue)
				return r;
                        json_parser_unwind(parser, c);
			parser->parser_switch = k_parsing_json;
			parser->token = k_number;
        } else {
                json_parser_set_error(parser, k_token_error,
                                      "Invalid character while parsing number");
                r = k_token_error;
	}

	return r;
}

static int32_t json_parser_feed_true(json_parser_t* parser, int c)
{
	int32_t r = json_parser_append(parser, c);
	if (r != k_continue)
		return r;

	if (parser->bufindex < 4) {
		if (memcmp(parser->buffer, "true", parser->bufindex) != 0) {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'true'");
			return k_token_error;
                }
	} else if (parser->bufindex == 4) {
		if (memcmp(parser->buffer, "true", 4) == 0) {
			parser->parser_switch = k_parsing_json;
			parser->token = k_true;
                        json_parser_reset_buffer(parser);
                        return k_continue;
		} else {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'true'");
			return k_token_error;
                }
	}
	return k_continue;
}

static int32_t json_parser_feed_false(json_parser_t* parser, int c)
{
	int32_t r = json_parser_append(parser, c);
	if (r != k_continue)
		return r;

	if (parser->bufindex < 5) {
		if (memcmp(parser->buffer, "false", parser->bufindex) != 0) {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'false'");
			return k_token_error;
                }

	} else if (parser->bufindex == 5) {
		if (memcmp(parser->buffer, "false", 5) == 0) {
			parser->parser_switch = k_parsing_json;
			parser->token = k_false;
                        json_parser_reset_buffer(parser);
                        return k_continue;
		} else {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'false'");
			return k_token_error;
                }
	}
	return k_continue;
}

static int32_t json_parser_feed_null(json_parser_t* parser, int c)
{
	int32_t r = json_parser_append(parser, c);
	if (r != k_continue)
		return r;

	if (parser->bufindex < 4) {
		if (memcmp(parser->buffer, "null", parser->bufindex) != 0) {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'null'");
			return k_token_error;
                }

	} else if (parser->bufindex == 4) {
		if (memcmp(parser->buffer, "null", 4) == 0) {
			parser->parser_switch = k_parsing_json;
			parser->token = k_null;
                        json_parser_reset_buffer(parser);
                        return k_continue;
		} else {
                        json_parser_set_error(parser, k_token_error,
                                              "Invalid character while parsing 'null'");
			return k_token_error;
                }
	}
	return k_continue;
}

static int32_t json_parser_feed_json(json_parser_t* parser, int c)
{
	int32_t r = k_continue;

	if (whitespace(c))
		return k_continue;

	switch (c) {
	case '{': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_object_start;
		break;

	case '}': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_object_end;
		break;

	case '[': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_array_start;
		break;

	case ']': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_array_end;
		break;

	case '"': 
	case '\'':
                parser->quote = c;
		parser->parser_switch = k_parsing_string;
		break;

	case 't': 
		parser->parser_switch = k_parsing_true;
                r = json_parser_feed_true(parser, c);
		break;

	case 'f': 
		parser->parser_switch = k_parsing_false;
                r = json_parser_feed_false(parser, c);
		break;

	case 'n': 
		parser->parser_switch = k_parsing_null;
                r = json_parser_feed_null(parser, c);
		break;
		
	case '-': case 'O': case '0': case '1': 
	case '2': case '3': case '4': case '5': 
	case '6': case '7': case '8': case '9': 
		parser->parser_switch = k_parsing_number;
		parser->numstate = _0_1;
                // we could do an unwind here. In that case
                // json_parser_feed_number will be called in the next
                // loop. We call json_parser_feed_number directly here
                // for speed optimization.
                // json_parser_unwind(parser, c)
                r = json_parser_feed_number(parser, c);
		break;

	case ',': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_comma;
		break;	

	case ':': 
		parser->parser_switch = k_parsing_json;
		parser->token = k_colon;
		break;

	case '\0':
		r = k_end_of_string;
		break;

	default: 
                json_parser_set_error(parser, k_token_error,
                                      "Invalid character while parsing json");
		r = k_token_error;
		break;
	}

	return r;
}

static int32_t json_parser_feed_one(json_parser_t* parser, int c)
{
        int32_t ret = k_token_error;

        parser->token = k_no_token;

	switch (parser->parser_switch) {
	case k_parsing_json: 
		ret = json_parser_feed_json(parser, c);
                break;

	case k_parsing_string: 
		ret = json_parser_feed_string(parser, c);
                break;

	case k_parsing_number:
		ret = json_parser_feed_number(parser, c);
                break;

	case k_parsing_false: 
		ret = json_parser_feed_false(parser, c);
                break;

	case k_parsing_true: 
		ret = json_parser_feed_true(parser, c);
                break;

	case k_parsing_null: 
		ret = json_parser_feed_null(parser, c);
                break;
    default:
        r_warn("json_parser_feed_one: unknown parser switch");
	}

    if (parser->token > k_no_token)
            ret = json_parser_token(parser, parser->token);

	return ret;
}

static inline int32_t _parser_done(json_parser_t* parser)
{
	return (parser->state_stack_top == 0
                && parser->value_stack_top == 0
                && parser->state_stack[parser->state_stack_top] == k_value_parsed);
}

int32_t json_parser_done(json_parser_t* parser)
{
	return _parser_done(parser);
}

json_object_t json_parser_result(json_parser_t* parser)
{
	return parser->value_stack[0];
}

static int json_parser_getc(json_parser_t* parser, input_read_t in, void *ptr)
{
        int c;
        if (parser->unwind_char != -1) {
                c = parser->unwind_char;
                parser->unwind_char = -1;
        } else {
                c = in(ptr);
                parser->colnum++;
                if (c == '\n') {
                        parser->linenum++;
                        parser->colnum = 0;
                }
        }
        return c;
}

static int json_parser_flush(json_parser_t* parser,
                             input_read_t in,
                             void *ptr)
{
        int err = -1;
        
        while (1) {
                int c = json_parser_getc(parser, in, ptr);
                if (c == -1) {
                        err = 0;
                        break;
                }
                if (!whitespace(c)) {
                        err = -1;
                        break;
                }
        }
        
        return err;
}

void json_format_error(char *s, int* err, char* errmsg, size_t len, const char *name, const json_parser_t* parser)
{
    if (err) *err = 1;
    if (errmsg) {
            snprintf(errmsg, len,
                     "json_parser_loop: %s:%d:%d  error: %s",
                     name, parser->linenum, parser->colnum, s);
            errmsg[len-1] = 0;
    }
}

static json_object_t json_parser_loop(json_parser_t* parser,
                                      const char *name,
                                      input_read_t in,
                                      void *ptr,
                                      int* err,
                                      char* errmsg,
                                      size_t len)
{
        int32_t r;
        int done = 0;
        
        if (errmsg)
                errmsg[0] = 0;
        if (err)
                *err = 0;

        do {
                
                int c = json_parser_getc(parser, in, ptr);

                // If we reached the end of the stream, call the
                // parser once more with a whitespace character to end
                // the parsing of single numbers.
                if (c == -1)
                        r = json_parser_feed_one(parser, ' ');
                else 
                        r = json_parser_feed_one(parser, c);
                if (r != 0) {
                        json_format_error(json_parser_errstr(parser), err, errmsg, len, name, parser);
                        return json_null();
                }
                
                done = _parser_done(parser);
                if (c == -1 && !done) {
                        json_format_error("The file is corrupt.", err, errmsg, len, name, parser);
                        return json_null();
                }
        } while (!done);
        
        if (json_parser_flush(parser, in, ptr)) {
                json_format_error(json_parser_errstr(parser), err, errmsg, len, name, parser);
                return json_null();
        }

        return json_parser_result(parser);
}

json_object_t json_load(const char* filename, int* err, char* errmsg, size_t len)
{
        json_parser_t* parser = json_parser_create();

        FILE* fp = fopen(filename, "r");
        if (fp == NULL) {
                *err = 1;
                snprintf(errmsg, len, "json_load: Failed to open the file: %s", filename);
                errmsg[len-1] = 0;
                json_parser_data_destroy(parser);
                return json_null();
        }


        json_object_t obj = json_parser_loop(parser, filename,
                                             file_input_read, fp,
                                             err, errmsg, len);

        fclose(fp);
        
        if (json_isnull(obj))
            json_parser_data_destroy(parser);
        else
            json_parser_destroy(parser);

        return obj;
}

json_object_t json_parser_eval_ext(json_parser_t* parser, const char* s,
                                   int* err, char* errmsg, size_t len)
{
        string_input_t in = { s, 0 };
	json_parser_reset(parser);
        return json_parser_loop(parser, "[string]", string_input_read, &in, err, errmsg, len);
}

json_object_t json_parser_eval(json_parser_t* parser, const char* s)
{
        int err;
        return json_parser_eval_ext(parser, s, &err, NULL, 0);
}

json_object_t json_parse_ext(const char* buffer, int* err, char* errmsg, size_t len)
{
        json_parser_t* parser;
        json_object_t obj = json_null();
        string_input_t in = { buffer, 0 };

        if (buffer != 0) {
                
                parser = json_parser_create();

                obj = json_parser_loop(parser, "[string]", string_input_read, &in,
                                       err, errmsg, len);

                if (json_isnull(obj))
                        json_parser_data_destroy(parser);
                else
                        json_parser_destroy(parser);
        }
        
        return obj;        
}
        
json_object_t json_parse(const char* buffer)
{
        int err;
        return json_parse_ext(buffer, &err, NULL, 0);
}

/******************************************************************************/

void json_cleanup()
{
        _delete(_null);
        _delete(_true);
        _delete(_false);
        _delete(_undefined);
}

