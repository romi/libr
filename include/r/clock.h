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
#ifndef _R_CLOCK_H_
#define _R_CLOCK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MICROSECONDS_IN_SECOND  (double)1000000.0

// Micro-seconds since UNIX epoch
uint64_t clock_timestamp();

// Seconds since UNIX epoch
double clock_time();

// Return date-time as string in the form of "2018-11-14 17:52:38"
char *clock_datetime(char *buf, int len, char sep1, char sep2, char sep3);
// Millisecond resolution.
char *clock_log_datetime(char *buf, int len, char sep1, char sep2, char sep3);

void clock_sleep(double seconds);
        
#ifdef __cplusplus
}
#endif

#endif // _R_CLOCK_H_
