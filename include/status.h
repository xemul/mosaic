#ifndef __MOSAIC_STATUS_H__
#define __MOSAIC_STATUS_H__
#include <stdbool.h>

struct mosaic;
void st_set_mounted(struct mosaic *, char *path);

int st_umount(struct mosaic *m, char *path, int (*cb)(struct mosaic *, char *));
#endif
