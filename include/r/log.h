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
#include <stdio.h>

#ifndef _R_LOG_H
#define _R_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define R_DEBUG 0
#define R_INFO 1
/* App can continue: */
#define R_WARNING 2
/* App cannot continue: */
#define R_ERROR 3
/* Call the developer */
#define R_PANIC 4

int r_log_init();
void r_log_cleanup();

void r_err(const char* format, ...);
void r_warn(const char* format, ...);
void r_info(const char* format, ...);
void r_debug(const char* format, ...);
void r_panic(const char* format, ...);

int r_log_get_level();
void r_log_set_level(int level);

int r_log_set_file(const char* path);
const char *r_log_get_file();

void r_log_set_app(const char* name);

typedef void (*log_writer_t)(void *userdata, const char* s);

void r_log_set_writer(log_writer_t callback, void *userdata);
void r_log_get_writer(log_writer_t *callback, void **userdata);

#ifdef __cplusplus
}
#endif

#endif // _R_LOG_H
