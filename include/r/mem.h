
#ifndef _R_MEM_H_
#define _R_MEM_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int mem_init(int *argc);
void mem_cleanup();

void *safe_malloc(size_t size, int zero);
void safe_free(void *ptr);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_realloc(void *ptr, size_t size);
char *safe_strdup(const char *s);

#define r_new(_type)         ((_type*)safe_malloc(sizeof(_type), 1))
#define r_delete(_p)         safe_free(_p)
#define r_alloc(_size)       safe_malloc(_size, 0)
#define r_calloc(_n,_size)   safe_calloc(_n,_size)
#define r_realloc(_p,_size)  safe_realloc(_p,_size)
#define r_free(_p)           safe_free(_p)
#define r_array(_type,_len)  ((_type*)safe_malloc((_len) * sizeof(_type)))
#define r_strdup(_s)         safe_strdup(_s)

#ifdef __cplusplus
}
#endif

#endif // _R_MEM_H_
