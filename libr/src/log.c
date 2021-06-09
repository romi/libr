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
#define _DEFAULT_SOURCE
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "r.h"

#define ONE_KB_BUFFER (1024)
#define LOG_INFO_SIZE (48) // Time, Type, Name, thread_id

static int _log_level = R_DEBUG;
static FILE* _log_file = NULL;
static char* _log_app = NULL;
static char* _log_path = NULL;
static mutex_t *_mutex = NULL;
static log_writer_t _log_writer = NULL;
static void *_log_write_data = NULL;

static char log_buffer[ONE_KB_BUFFER];

char *clock_log_datetime(char *buf, size_t len, char sep1, char sep2, char sep3)
{
        struct tm r;
        struct timeval tv;
        struct timespec ts;
        clock_gettime_wrapper(CLOCK_REALTIME, &ts);

        tv.tv_sec = ts.tv_sec;
    tv.tv_usec =ts.tv_nsec / 1000;

        localtime_r_wrapper(&tv.tv_sec, &r);

        long milliseconds = tv.tv_usec/1000;

        snprintf(buf, len, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d.%03ld",
                 1900 + r.tm_year, sep1, 1 + r.tm_mon, sep1, r.tm_mday,
                 sep2,
                 r.tm_hour, sep3, r.tm_min, sep3, r.tm_sec, milliseconds);
        return buf;
}

int r_log_init()
{
    _mutex = new_mutex();
    return (_mutex == NULL)? -1 : 0;
}

void r_log_set_app(const char* name)
{
        if (_log_app != NULL) 
                r_free(_log_app);   
        _log_app = r_strdup(name);
}

int r_log_set_file(const char* path)
{
        int err = -1;
        
        r_info("Changing log to '%s'", path);
        
        if (_mutex) mutex_lock(_mutex);

        if (_log_path != NULL) {
                r_free(_log_path);
                _log_path = NULL;
        }
        if (_log_file != NULL && _log_file != stdout) {
                fclose(_log_file);
                _log_file = NULL;
        }
        
        _log_path = r_strdup(path);
        
        if (_log_path == NULL) {
                fprintf(stderr, "log_set_file: out of memory");
                err = -1;
        
        } else {
                if (strcmp(_log_path, "-") == 0) {
                        _log_file = stdout;
                        err = 0;
                        
                } else {
                        _log_file = fopen(_log_path, "a");
                        if (_log_file == NULL) {
                                fprintf(stderr, "Failed to open logfile at '%s'\n",
                                        _log_path);
                                _log_file = stdout;
                                err = -1;
                        } else {
                                err = 0;
                        }
                }
        }
        if (_mutex) mutex_unlock(_mutex);
        
        r_info("Log started ----------------------------------------");

        return err;
}

const char *r_log_get_file()
{
        return _log_path;
}

void r_log_cleanup()
{
        if (_log_app != NULL) {
                r_free(_log_app);
                _log_app = NULL;
        }
        if (_log_path != NULL) {
                r_free(_log_path);
                _log_path = NULL;
        }
        if (_log_file != NULL && _log_file != stdout) {
                fclose(_log_file);
                _log_file = NULL;
        }
        if (_mutex != NULL) {
                delete_mutex(_mutex);
                _mutex = NULL;
        }

        _log_level = R_DEBUG;
        _log_writer = NULL;
        _log_write_data = NULL;
}

int r_log_get_level()
{
        return _log_level;
}

void r_log_set_level(int level)
{
        _log_level = level;
        if (_log_level < R_DEBUG) 
                _log_level = R_DEBUG;
        else if (_log_level > R_ERROR)
                _log_level = R_ERROR;
}

void r_log_set_writer(log_writer_t callback, void *userdata)
{
    if (_mutex) mutex_lock(_mutex);
        _log_write_data = userdata;
        _log_writer = callback;
    if (_mutex) mutex_unlock(_mutex);
}

void r_log_get_writer(log_writer_t *callback, void **userdata)
{
        *callback = _log_writer;
        *userdata = _log_write_data;
}

static void check_output_set()
{
    if (_log_file == NULL)
        _log_file = stdout;
}

static void r_log_write(const char* s)
{
        check_output_set();

        log_writer_t callback = NULL;
        void *userdata = NULL;
                
        if (_mutex) mutex_lock(_mutex);

        fprintf(_log_file, "%s\n", s);
        fflush(_log_file);

        if (_log_writer) {
                 callback = _log_writer;
                 userdata = _log_write_data;
        }

        if (_mutex) mutex_unlock(_mutex);

        // Call the external log_writer outside of the mutex. This
        // avoids a deadlock in case the callback uses one of the log
        // functions.
        if (callback) callback(userdata, s);
}

static void log_(int level, const char* s)
{

        static char buffer[ONE_KB_BUFFER + LOG_INFO_SIZE];
        static char timestamp[256];
        const char* time = clock_log_datetime(timestamp, sizeof(timestamp), '-', ' ', ':');
        const char* type;
        const char* name = _log_app? _log_app : "?";
        switch (level) {
        case R_DEBUG: type = "DD"; break;
        case R_INFO: type = "II"; break;
        case R_WARNING: type = "WW"; break;
        case R_ERROR: type = "EE"; break;
        case R_PANIC: type = "!!"; break;
        default: type = "??"; break;
        }

        rprintf(buffer, sizeof(buffer), "%s, %s, %s, 0x%x, %s", time, type, name, pthread_self(), s);
        r_log_write(buffer);

        if (level == R_PANIC && _log_file != stdout) {
                fprintf(stdout, "%s\n", buffer);
                fflush(stdout);
        }
}


static int log_printf(char* buffer, size_t bufflen, const char* format, va_list ap)
{
        memset(buffer, 0, bufflen);
        vsnprintf(buffer, bufflen, format, ap);
        return 0;
}

void r_err(const char* format, ...)
{
        va_list ap;
        va_start(ap, format);
        log_printf(log_buffer, ONE_KB_BUFFER, format, ap);
        va_end(ap);

        log_(R_ERROR, log_buffer);
}

void r_warn(const char* format, ...)
{
        va_list ap;
        if (_log_level > R_WARNING)
                return;
        va_start(ap, format);
        log_printf(log_buffer, ONE_KB_BUFFER, format, ap);
        va_end(ap);

        log_(R_WARNING, log_buffer);
}

void r_info(const char* format, ...)
{
        va_list ap;
        if (_log_level > R_INFO)
                return;
        va_start(ap, format);
        log_printf(log_buffer, ONE_KB_BUFFER, format, ap);
        va_end(ap);

        log_(R_INFO, log_buffer);
}

void r_debug(const char* format, ...)
{
        va_list ap;
        if (_log_level > R_DEBUG) 
                return;
        va_start(ap, format);
        log_printf(log_buffer, ONE_KB_BUFFER, format, ap);
        va_end(ap);

        log_(R_DEBUG, log_buffer);
}

void r_panic(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    log_printf(log_buffer, ONE_KB_BUFFER, format, ap);
    va_end(ap);

    log_(R_PANIC, log_buffer);
}
