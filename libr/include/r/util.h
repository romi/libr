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
#ifndef _R_UTIL_H
#define _R_UTIL_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define rstreq(_s1, _s2) ((_s1) != NULL && (_s2) != NULL && strcmp(_s1,_s2)==0)
char *rprintf(char *buffer, size_t len, const char *format, ...);
int r_random(void *buf, size_t buflen);
char *r_uuid();
        
#ifdef __cplusplus
}
#endif

#endif // _R_UTIL_H

