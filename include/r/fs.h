
#ifndef _R_FS_H_
#define _R_FS_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

int path_exists(const char *path);
int path_is_absolute(const char *path);
int path_make_absolute(const char *path, char *buffer, int len);
list_t* path_break(const char *path);
int path_glue(list_t* elements, int absolute, char *buffer, int len);
void path_delete(list_t* list);
int path_chown(const char *path, const char *user);


int is_file(const char *path);
int is_directory(const char *path);

enum {
        FS_BACKUP = 1,
        FS_LOCK = 2
};

int file_store(const char *path, char *data, int len, int flags);

list_t *directory_list(const char *path);
int directory_create(const char *path);

#ifdef __cplusplus
}
#endif

#endif  /* _R_FS_H_ */
