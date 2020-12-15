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

#ifndef _R_FS_H_
#define _R_FS_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

int path_exists(const char *path);
int path_is_absolute(const char *path);
int path_make_absolute(const char *path, char *buffer, int len);
list_t* path_break(const char *path);
int path_glue(list_t* elements, int absolute, char *buffer, int len);
void path_delete(list_t* list);
int path_chown(const char *path, const char *user);

int is_file(const char *path);
int is_directory(const char *path);

enum {
        FS_BACKUP = 1,
        FS_LOCK = 2
};

int file_store(const char *path, char *data, int len, int flags);

list_t *directory_list(const char *path);
int directory_create(const char *path);

#ifdef __cplusplus
}
#endif

#endif  /* _R_FS_H_ */
