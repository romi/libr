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
#ifndef R_MEMBUFFER_H
#define R_MEMBUFFER_H

class membuffer
{
public:
    membuffer() = default;
    virtual ~membuffer() = default;
};

//
//typedef struct _membuf_t membuf_t;
//
//struct _membuf_t
//{
//    char* buffer;
//    int length;
//    int index;
//    mutex_t *mutex;
//};
//
//membuf_t *new_membuf();
//void delete_membuf(membuf_t *b);
//
//void membuf_put(membuf_t *b, char c);
//void membuf_append(membuf_t *b, const char *data, int len);
//void membuf_append_zero(membuf_t *b);
//
///**
// *   Appends a zero-terminated string to the buffer. The string should
// *   be less than 32kB long and will be (silently) truncated if it is
// *   longer. To append larger strings, use membuf_append.
// *
// *   @b: the membuf
// *   @s: the (short) string to be appended.
// */
//void membuf_append_str(membuf_t *b, const char *s);
//int membuf_printf(membuf_t *b, const char* format, ...);
//int membuf_vprintf(membuf_t *b, const char* format, va_list ap);
//void membuf_print_obj(membuf_t *b, json_object_t obj);
//
//int membuf_available(membuf_t *b);
//void membuf_assure(membuf_t *b, int size);
//void membuf_clear(membuf_t *b);
//
//char* membuf_data(membuf_t *b);
//int membuf_len(membuf_t *b);
//
//void membuf_lock(membuf_t *b);
//void membuf_unlock(membuf_t *b);
//mutex_t *membuf_mutex(membuf_t *b);
//
//int membuf_size(membuf_t *b);
//
//// Use with caution!
//void membuf_set_len(membuf_t *b, int len);


#endif // _R_MEMBUF_H_
