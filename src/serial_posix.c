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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "r.h"

struct _serial_t
{
        char *device;
        int speed;
        int fd;
        membuf_t *out;
        int nextchar;
        mutex_t *mutex;
        int errors;
        int quit;
};

static int open_serial(const char *device, int speed, int reset);

// See also:
//   http://www.delorie.com/gnu/docs/glibc/libc_364.html
//   https://www.cmrr.umn.edu/~strupp/serial.html
//   https://www.gnu.org/software/libc/manual/html_node/Canonical-or-Not.html
//   https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
//   https://stackoverflow.com/questions/41224496/working-with-linux-serial-port-in-c-not-able-to-get-full-data/41252962#41252962
//   https://forum.arduino.cc/index.php/topic,28167.0.html
static int open_serial(const char *device, int speed, int reset)
{
        struct termios tty;
        int fd;
        int speed_constant;

        switch (speed) {
        case 9600: speed_constant = B9600; break;
        case 19200: speed_constant = B19200; break;
        case 38400: speed_constant = B38400; break;
        case 57600: speed_constant = B57600; break;
        case 115200: speed_constant = B115200; break;
        default:
                r_err("open_serial: only the following speeds are valid: "
                        "9600, 19200, 38400, 57600, 115200");
                return -1;                
        }

        fd = open_wrapper(device, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
                r_err("open_serial: error %d opening %s: %s",
                        errno, device, strerror(errno));
                return -1;
        }

        memset(&tty, 0, sizeof(tty));
        if (tcgetattr(fd, &tty) != 0) {
                r_err("open_serial: error %d from tcgetattr", errno);
                close_wrapper(fd);
                return -1;
        }

        // cflag        
        // 8n1 (8bit, no parity, 1 stopbit)
        // CLOCAL: ignore modem controls
        // CREAD: enable reading
        // CS8: 8-bit characters
    // ToDo: Whats going on here? Should thie or in these values? HUPCL
        tty.c_cflag = CLOCAL | CREAD | CS8;
        if (!reset)
                tty.c_cflag &= ~HUPCL;   // disable hang-up-on-close to avoid reset

        // lflag
        // no signaling chars; no echo; ...
        tty.c_lflag = ICANON;    // enable canonical processing
        
        // iflag
        // no parity check; no break processing; don't map NL to CR,
        // CR to NL, uppercase to lowercase; ring bell; shut off
        // xon/xoff ctrl; ...
        tty.c_iflag = IGNCR;    // ignore carriage-return '\r'
        
        // oflag
        // no remapping; no delays; no post-processing
        tty.c_oflag = 0;                

        // Use for non-canonical input
        /* tty.c_cc[VMIN]  = 0; */
        /* tty.c_cc[VTIME] = 5; */
        
        cfsetspeed(&tty, speed_constant);
        tcflush(fd, TCIOFLUSH);

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                r_err("open_serial: error %d from tcsetattr", errno);
                close_wrapper(fd);
                return -1;
        }
        return fd;
}

serial_t *new_serial(const char *device, int speed, int reset)
{
        serial_t *s;

        // First check whether we can open the serial connection.
        int fd = open_serial(device, speed, reset);
        if (fd == -1) return NULL;
        
        // Create the object
        s = r_new(serial_t);
        
        s->fd = fd;
        s->device = r_strdup(device);
        s->speed = speed;
        s->out = new_membuf();
        s->nextchar = -1;
        s->errors = 0;
        s->quit = 0;
        
        s->mutex = new_mutex();
        
        if (s->device == NULL || s->out == NULL) {
                delete_serial(s);
                return NULL;
        }
        
        return s;
}

void delete_serial(serial_t *s)
{
        if (s) {
                s->quit = 1;
                serial_lock(s);
                r_free(s->device);
                delete_membuf(s->out);
                close_wrapper(s->fd);
                s->fd = -1;
                serial_unlock(s);
                delete_mutex(s->mutex);
                r_delete(s);
        }
}

static int serial_peek(serial_t *s)
{
        if (s->nextchar == -1)
                s->nextchar = serial_get(s);
        return s->nextchar;        
}

int serial_get(serial_t *s)
{
        unsigned char c;

        if (s->fd == -1) return -1;

        if (s->nextchar != -1) {
                int n = s->nextchar;
                s->nextchar = -1;
                return n;
        }

        while (!s->quit) {
                ssize_t n = read(s->fd, &c, 1);
                if (n == 1) {
                        printf("serial_get: '%c'\n", c);
                        return (int) c;
                }
                if (n == -1) {
                        // ToDo: Should this be r_err?
                        perror("serial_get");
                        return -1;
                }
                printf("serial_get: sleep (n=%d)\n", (int) n);
                usleep_wrapper(1000);
        }
        return -1;
}

int serial_read(serial_t *s, char *buf, int len)
{
        int n = 0;
        if (s->fd == -1)
                return -1;
        
        while (n < len) {
                if (s->quit)
                        return -1;
                ssize_t m = read(s->fd, buf + n, len - n);
                if (m == -1) {
                        perror("serial_read");
                        return -1;
                }
                n += m;
        }
        return 0;
}

static int arduino_debug_string(membuf_t *buffer)
{
    char *data = membuf_data(buffer);
    if (membuf_len(buffer) >= 3
        && strncmp(data, "#!", 2) == 0)
    {
        r_debug("serial_readline Arduino: %s", data);
        membuf_clear(buffer);
        return 1;
    }
    return 0;
}

const char *serial_readline(serial_t *s, membuf_t *buffer)
{
        if (s->fd == -1)
                return NULL;
        
        membuf_clear(buffer);
        
        while (!s->quit) {
                int c = serial_get(s);

                if (c == -1) {
                        return NULL;
                        
                } else if (c == '\r') {
                        if (serial_peek(s) == '\n')
                                c = serial_get(s);
                        membuf_append_zero(buffer);
                        if (!arduino_debug_string(buffer)){
                            return membuf_data(buffer);
                        }

                } else if (c == '\n') {
                        membuf_append_zero(buffer);
                    if (!arduino_debug_string(buffer)){
                        return membuf_data(buffer);
                    }
                } else {
                        membuf_put(buffer, (char) (c & 0xff));
                }
        }

        return NULL;
}

int serial_put(serial_t *s, char c)
{
        if (s->fd == -1) return -1;
        while (!s->quit) {
                ssize_t m = write(s->fd, &c, 1);
                if (m == 1)
                        return 0;
                if (m < 0) {
                        perror("serial_put");
                        return -1;
                }
        }
        return 0;
}

int serial_write(serial_t *s, const char *buf, int len)
{
        int n = 0;
	
        if (s->fd == -1)
                return -1;
	
        while (n < len) {
                if (s->quit)
                        return -1;
                ssize_t m = write(s->fd, buf + n, len - n);
                if (m < 0) {
                        perror("serial_write");
                        return -1;
                }
                n += m;
        }
        return 0;
}

int serial_print(serial_t *s, const char *line)
{
        return serial_write(s, line, strlen(line));
}

//int serial_println(serial_t *s, const char *line)
//{
//        if (serial_write(s, line, strlen(line)) != 0)
//                return -1;
//        int ret = serial_write(s, "\r\n", 2);
//	return ret;
//}

int serial_println(serial_t *s, const char *line)
{
    return serial_printf(s, "%s\r\n", line);
}

int serial_printf(serial_t *s, const char *format, ...)
{
    va_list ap;
    int r;

    membuf_clear(s->out);

    va_start(ap, format);
    r = membuf_vprintf(s->out, format, ap);
    va_end(ap);

    if (r < 0) {
        r_err("serial_printf: membuf_vprintf returned an error");
        return -1;
    }

    return serial_write(s, membuf_data(s->out), membuf_len(s->out));
}

//int serial_printf(serial_t *s, const char *format, ...)
//{
//        va_list ap;
//        int len, r;
//
//        va_start(ap, format);
//        len = vsnprintf(NULL, 0, format, ap);
//        va_end(ap);
//
//        if (len < 0) {
//                r_err("serial_printf: vsnprintf returned an error");
//                return -1;
//        }
//
//        membuf_clear(s->out);
//        r = membuf_assure(s->out, len+1);
//        if (r != 0)
//                return -1;
//
//        va_start(ap, format);
//        membuf_vprintf(s->out, format, ap);
//        va_end(ap);
//
//        return serial_write(s, membuf_data(s->out), membuf_len(s->out));
//}

void serial_lock(serial_t *s)
{
        mutex_lock(s->mutex);
}

void serial_unlock(serial_t *s)
{
        mutex_unlock(s->mutex);
}

const char *serial_command_send(serial_t *s, membuf_t *message, const char *cmd)
{
        int r;
        const char *reply = NULL;

	//r_debug("serial_command_send: %s", cmd);

        if (s == NULL)
                return NULL;
        
        serial_lock(s);
        
        r = serial_println(s, cmd);
        if (r == 0)
                reply = serial_readline(s, message);

        if (r != 0) {
                s->errors++;
                r_err("serial_command_send: failed to send the command");
        } else if (reply == NULL) {
                s->errors++;
                r_err("serial_command_send: reply == NULL");
        } else if (strncmp(reply, "ERR", 3) == 0) {
                s->errors++;
                r_err("serial_command_send: %s", reply);
        } 

        serial_unlock(s);

	/* r_debug("serial_command_send: reply %.*s", */
        /*           membuf_len(message), membuf_data(message)); */
	
        return reply;
}

//const char *serial_command_sendf(serial_t *s, membuf_t *message, const char *format, ...)
//{
//        va_list ap;
//        int len, r;
//        const char *reply = NULL;
//
//        if (s == NULL)
//                return NULL;
//
//        // Check how long the buffer should be.
//        va_start(ap, format);
//        len = vsnprintf(NULL, 0, format, ap);
//        va_end(ap);
//
//        if (len < 0) {
//                r_err("serial_command_sendf: vsnprintf returned an error");
//                return NULL;
//        }
//
//        serial_lock(s);
//
//        membuf_clear(s->out);
//        membuf_assure(s->out, len+1);
//
//        va_start(ap, format);
//        membuf_vprintf(s->out, format, ap);
//        va_end(ap);
//
//	//r_debug("serial_command_sendf: %s", membuf_data(s->out));
//
//        //printf("serial: sending command: %s\n", membuf_data(s->out));
//        r = serial_println(s, membuf_data(s->out));
//        if (r != 0)
//                goto unlock_and_return;
//
//        //printf("serial: reading reply\n");
//        reply = serial_readline(s, message);
//
//unlock_and_return:
//        serial_unlock(s);
//
//	//r_debug("serial_command_sendf: reply %s", reply);
//
//        return reply;
//}

const char *serial_command_sendf(serial_t *s, membuf_t *message, const char *format, ...)
{
    va_list ap;
    int r;
    const char *reply = NULL;

    if (s == NULL)
        return NULL;

    serial_lock(s);
    va_start(ap, format);
    membuf_vprintf(s->out, format, ap);
    va_end(ap);

    //r_debug("serial_command_sendf: %s", membuf_data(s->out));
    //printf("serial: sending command: %s\n", membuf_data(s->out));
    r = serial_println(s, membuf_data(s->out));
    if (r == 0) {
        //printf("serial: reading reply\n");
        reply = serial_readline(s, message);
    }
    serial_unlock(s);
    //r_debug("serial_command_sendf: reply %s", reply);

    return reply;
}


int serial_flush(serial_t *s)
{
        int r = tcflush(s->fd, TCIOFLUSH);
        if (r != 0) {
                r_err("serial_flush: %s", strerror(errno));
        }
        return r;
}
