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
#ifndef _R_SERIAL_H_
#define _R_SERIAL_H_

#include <termios.h>
#include "membuf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _serial_t serial_t;

serial_t *new_serial(const char *device, int speed, int reset);
void delete_serial(serial_t *s);
int get_termios(serial_t *s, struct termios *tty);
int set_termios(serial_t *s, struct termios myTermios);

int serial_get(serial_t *s);
int serial_read(serial_t *s, char *buf, size_t len);
const char *serial_readline(serial_t *s, membuf_t *buffer);
int serial_read_timeout(serial_t *s, char *buf, size_t len, int timeout_ms);

// returns 0: no error, -1: error
// These functions are not protected by a mutex. 
int serial_put(serial_t *s, char c);
int serial_write(serial_t *s, const char *buf, size_t Slen);
int serial_print(serial_t *s, const char *line);
int serial_println(serial_t *s, const char *line);
int serial_printf(serial_t *s, const char *format, ...);

/** serial_send_command() is a combination of a println and a
 * readline. The sending and reading are performed as a single
 * operation protected by a mutex to avoid being interrupted by
 * another thread.
 *
 * s: The serial connection to the device.
 * message: Upon return from this function, this membuf contains the reply of the device. 
 * cmd: This null-terminted string contains the command to be sent.
 */
const char *serial_command_send(serial_t *s, membuf_t *message, const char *cmd);
const char *serial_command_sendf(serial_t *s, membuf_t *message, const char *format, ...);

int serial_flush(serial_t *s);

void serial_lock(serial_t *s);
void serial_unlock(serial_t *s);

#ifdef __cplusplus
}
#endif

#endif // _R_SERIAL_H_
