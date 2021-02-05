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
#include <stdlib.h>
#include "r.h"

#define ONE_MB (1024*1024)
//#define MEM_DIAGNOSTICS

static out_of_memory_handler_t _out_of_memory_handler = NULL;

#if defined MEM_DIAGNOSTICS
#define SGC_USE_THREADS 1
#include <sgc.h>

int mem_init(int *argc, out_of_memory_handler_t handler)
{
	if (sgc_init(argc, 0) != 0) {
                r_err("Failed the initialise the SGC memory heap");
                return -1;
        }
        _out_of_memory_handler = handler;
        return 0;
}

static int print_memory_leak(int op, void* ptr,
                             unsigned char type,
                             int counter, int size)
{
        if (op == 3) {
                printf("Memory leak: counter=%d, size=%d\n", counter, size);
        } 
        return 1;
}

void mem_cleanup()
{
        r_info("Scanning for memory leaks");
        sgc_search_memory_leaks(print_memory_leak);
        sgc_cleanup();
}

void *safe_malloc(size_t size, int zero)
{
        if (size == 0) {
                r_warn("safe_malloc: size == 0");
                return NULL;
        }
        
        void *ptr = sgc_new_object(size, SGC_ZERO, 0);

        if (ptr == NULL && _out_of_memory_handler != NULL) {
                // Free some memory
                _out_of_memory_handler();
        
                // Second attempt
                ptr = sgc_new_object(size, SGC_ZERO, 0);
        }

        if (ptr == NULL) {
                r_panic("safe_malloc: out of memory");
                exit_wrapper(1);
        }

        return ptr;
}

void safe_free(void *ptr)
{
        if (ptr == NULL)
                r_warn("safe_free: ptr == NULL");
        else
                sgc_free_object(ptr);
}

void *safe_calloc(size_t nmemb, size_t size)
{
        return sgc_new_object(nmemb * size, SGC_ZERO, 0);
}

void *safe_realloc(void *ptr, size_t size)
{
        if (size == 0)
                // Not an error
                r_warn("safe_realloc: size == 0"); 

        ptr = sgc_resize_object(ptr, size, 0, 0);
        
        if (size > 0
            && ptr == NULL
            && _out_of_memory_handler != NULL) {
                // Free some memory
                _out_of_memory_handler();
        
                // Second attempt
                ptr = sgc_resize_object(ptr, size, 0, 0);
        }

        if (size > 0 && ptr == NULL) {
                r_panic("safe_realloc: out of memory");
                exit_wrapper(1);
        }
        
        return ptr;
}

#else

int mem_init(int *argc __attribute__((unused)), out_of_memory_handler_t handler)
{
        _out_of_memory_handler = handler;
        return 0;
}

void mem_cleanup()
{
}

void *safe_malloc(size_t size, int zero)
{
        if (size == 0) {
                r_warn("safe_malloc: size == 0");
                return NULL;
        }
        
        void *ptr = malloc_wrapper(size);
        
        if (ptr == NULL && _out_of_memory_handler != NULL) {
                // Free some memory
                _out_of_memory_handler();
        
                // Second attempt
                ptr = malloc_wrapper(size);
        }

        if (ptr == NULL) {
                r_panic("safe_malloc: out of memory");
                exit_wrapper(1);
        }
        
        if (zero)
                memset_wrapper(ptr, 0, size);
        
        return ptr;
}

void safe_free(void *ptr)
{
        free_wrapper(ptr);
}

void *safe_calloc(size_t nmemb, size_t size)
{
        return safe_malloc(nmemb * size, 1);
}

void *safe_realloc(void *ptr, size_t size)
{
        if (size == 0)
                // Not an error
                r_warn("safe_realloc: size == 0"); 

        ptr = realloc_wrapper(ptr, size);
        
        if (size > 0
            && ptr == NULL
            && _out_of_memory_handler != NULL) {
                // Free some memory
                _out_of_memory_handler();
        
                // Second attempt
                ptr = realloc_wrapper(ptr, size);
        }

        if (size > 0 && ptr == NULL) {
                r_panic("safe_realloc: out of memory");
                exit_wrapper(1);
        }
        
        return ptr;
}

#endif // MEM_DIAGNOSTICS

char *safe_strdup(const char *s)
{
        if (s == NULL) {
                r_warn("safe_strdup: s == NULL!"); 
                return NULL;
        }
        size_t len = 0;
        while (s[len] != 0) {
                len++;
                if (len >= ONE_MB) {
                        r_warn("safe_strdup: strlen(s) > 1MB! Too big."); 
                        return NULL;
                }
        }
        char *t = r_alloc(len + 1);
        memcpy_wrapper(t, s, len+1);
        return t;
}
