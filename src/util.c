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
#include <sys/syscall.h>
#include "r.h"

char *rprintf(char *buffer, int buflen, const char *format, ...)
{
        int len;
        va_list ap;
        
        va_start(ap, format);
        len = vsnprintf(NULL, 0, format, ap);
        va_end(ap);

        if (len < 0 || buflen < len+1)
                return NULL;
        
        va_start(ap, format);
        len = vsnprintf(buffer, buflen, format, ap);
        va_end(ap);

        if (len < 0)
                return NULL;
        
        return buffer;
}

int r_random(void *buf, size_t buflen)
{
        int flags = 0; 
        return (int) syscall(SYS_getrandom, buf, buflen, flags);
}

char *r_uuid()
{
        uint8_t b[16];
        r_random(&b, sizeof(b));

        // RFC 4122 section 4.4
        b[6] = 0x40 | (b[6] & 0x0f);
        b[8] = 0x80 | (b[8] & 0x3f);
        
        char s[37];
        snprintf(s, 37,
                 "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                 b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
                 b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);

        return r_strdup(s);
}
