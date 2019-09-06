#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <uuid/uuid.h>
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
        return (int)syscall(SYS_getrandom, buf, buflen, flags);
}

char *r_uuid()
{
        uuid_t uuid;
        char s[37];
        uuid_generate(uuid);
        uuid_unparse_lower(uuid, s);
        return r_strdup(s);
}
