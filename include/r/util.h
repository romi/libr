#ifndef _R_UTIL_H
#define _R_UTIL_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define rstreq(_s1, _s2) ((_s1) != NULL && (_s2) != NULL && strcmp(_s1,_s2)==0)
char *rprintf(char *buffer, int len, const char *format, ...);
int r_random(void *buf, size_t buflen, unsigned int flags);
char *r_uuid();
        
#ifdef __cplusplus
}
#endif

#endif // _R_UTIL_H

