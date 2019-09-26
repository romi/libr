#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "r.h"

char *rprintf(char *buffer, int buflen, const char *format, ...)
{
        int len;
        va_list ap;
        int ret;
        
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
