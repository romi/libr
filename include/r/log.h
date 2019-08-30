#include <stdio.h>

#ifndef _R_LOG_H
#define _R_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define R_DEBUG 0
#define R_INFO 1
/* App can continue: */
#define R_WARNING 2
/* App cannot continue: */
#define R_ERROR 3
/* Call the developer */
#define R_PANIC 4


void r_err(const char* format, ...);
void r_warn(const char* format, ...);
void r_info(const char* format, ...);
void r_debug(const char* format, ...);
void r_panic(const char* format, ...);

int r_log_get_level();
void r_log_set_level(int level);

int r_log_set_file(const char* path);
const char *r_log_get_file();

void r_log_set_app(const char* name);

typedef void (*log_writer_t)(void *userdata, const char* s);

void r_log_set_writer(log_writer_t callback, void *userdata);
void r_log_get_writer(log_writer_t *callback, void **userdata);

#ifdef __cplusplus
}
#endif

#endif // _R_LOG_H
