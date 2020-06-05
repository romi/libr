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

#include "r.h"

#define KB_32 (1024 * 32)

static void membuf_grow(membuf_t *b)
{
        if (b) {
                int len = 2 * b->length;
                if (len == 0)
                        len = 128;
                b->buffer = r_realloc(b->buffer, len);
                b->length = len;
        }
}

static void membuf_grow_to_fit(membuf_t *b, int len)
{
        if (b) {
                while (b->index + len > b->length)
                        membuf_grow(b);
        }
}

membuf_t *new_membuf()
{
        membuf_t* membuf = r_new(membuf_t);
        membuf->mutex = new_mutex();
        return membuf;
}

void delete_membuf(membuf_t *b)
{
        if (b) {
                delete_mutex(b->mutex);
                r_free(b->buffer);
                r_delete(b);
        }
}

void membuf_put(membuf_t *b, char c)
{
        if (b) {
                membuf_grow_to_fit(b, 1);
                b->buffer[b->index++] = c;
        }
}

void membuf_lock(membuf_t *b)
{
        if (b)
                mutex_lock(b->mutex);
}

void membuf_unlock(membuf_t *b)
{
        if (b)
                mutex_unlock(b->mutex);
}

mutex_t *membuf_mutex(membuf_t *b)
{
        return b == NULL? NULL : b->mutex;
}

void membuf_append(membuf_t *b, const char *data, int len)
{
        if (b) {
                membuf_grow_to_fit(b, len);
                memcpy(b->buffer + b->index, data, len);
                b->index += len;
        }
}

void membuf_append_zero(membuf_t *b)
{
        membuf_put(b, 0);
}

void membuf_append_str(membuf_t *b, const char *s)
{
        size_t lens = strnlen(s, KB_32);
        if (lens == KB_32)
                r_warn("membuf_append_str: string truncated to 32kb");
        membuf_append(b, s, lens);
}

void membuf_clear(membuf_t *b)
{
        if (b) {
                memset(b->buffer, 0, b->length);
                b->index = 0;
        }
}

char* membuf_data(membuf_t *b)
{
        return b == NULL? NULL : b->buffer;
}

int membuf_len(membuf_t *b)
{
        return b == NULL? 0 : b->index;
}

void membuf_set_len(membuf_t *b, int len)
{
        if (b) {
                if (len <= b->length)
                        b->index = len;
        }
}

int membuf_available(membuf_t *b)
{
        return b == NULL? 0 : b->length - b->index;
}

int membuf_size(membuf_t *b)
{
        return b == NULL? 0 : b->length;
}

void membuf_assure(membuf_t *b, int size)
{
        membuf_grow_to_fit(b, size);
}

int membuf_printf(membuf_t *b, const char* format, ...)
{
        va_list ap;
        int ret;
        va_start(ap, format);
        ret = membuf_vprintf(b, format, ap);
        va_end(ap);
        return ret;
}

int membuf_vprintf(membuf_t* b, const char* format, va_list ap)
{
        int err = -1;
        int len;
        va_list ap_copy;
        va_copy(ap_copy, ap);

        len = vsnprintf(NULL, 0, format, ap);

        if (len >= 0) {
                membuf_assure(b, len+1);

                int available = membuf_available(b);
                
                len = vsnprintf(b->buffer + b->index, available, format, ap_copy);
                if (len >= 0 && len < available) {
                        b->index += len;
                        err = 0;
                }
        }
        
        va_end(ap_copy);
        
        return err;
}

static int32 membuf_json_writer(void* userdata, const char* s, int32 len)
{
        membuf_t *b = (membuf_t *) userdata;
        membuf_append(b, s, len);
        return 0;
}

void membuf_print_obj(membuf_t *b, json_object_t obj)
{
        json_serialise(obj, 0, membuf_json_writer, b);
}
