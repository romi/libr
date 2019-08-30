
#ifndef _R_FS_H_
#define _R_FS_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

int is_absolute(const char *path);
int make_absolute_path(const char *path, char *buffer, int len);

list_t *dir_list(const char *path);

#ifdef __cplusplus
}
#endif

#endif  /* _R_FS_H_ */
