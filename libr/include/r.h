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
#ifndef _R_H_
#define _R_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <r/mem.h>
#include <r/thread.h>
#include <r/fs.h>
#include <r/clock.h>
#include <r/membuf.h>
#include <r/list.h>
#include <r/log.h>
#include <r/util.h>
#include <r/json.h>
#include <r/serial.h>
#include <r/os_wrapper.h>

/**
 * Initializes the library.
 *
 * @handler: The function to be called when memory allocation
 * fails. This function should free up memory such that upon return
 * the memory request can be satisfied. If not, the application will
 * exit. The value of the handler can be NULL, in which case the
 * application will immediately exit on an out-of-memory situation.
 */        
int r_init(int *argc, out_of_memory_handler_t handler);

void r_cleanup();

#ifdef __cplusplus
}
#endif

#endif // _R_H_
