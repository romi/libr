#include <stdlib.h>
#include "mem.mock.h"

DEFINE_FAKE_VALUE_FUNC(void *, safe_malloc, size_t, int)
void *safe_malloc_custom_fake(size_t size, int zero)
{
    void *mem = malloc(size);
    if (zero)
        memset(mem, 0, size);
    return mem;
}


DEFINE_FAKE_VOID_FUNC(safe_free, void *)
void safe_free_custom_fake(void *mem)
{
    free(mem);
}
DEFINE_FAKE_VALUE_FUNC(void *, safe_realloc, void *, size_t)
DEFINE_FAKE_VALUE_FUNC(char *, safe_strdup, const char *)

