#ifndef __MOSAIC_STATUS_H__
#define __MOSAIC_STATUS_H__
#include <stdbool.h>

#define STATUS_DIR	"mosaic.status"

struct mosaic;
struct tessera;

void st_set_mounted(struct mosaic *, char *path);
void st_set_mounted_t(struct tessera *t, char *age, char *path);

int st_umount(struct mosaic *m, char *path, int (*cb)(struct mosaic *, char *));
int st_umount_t(struct tessera *t, char *age, char *path, int (*cb)(struct tessera *t, char *age, char *));

int st_is_mounted(struct mosaic *);
int st_is_mounted_t(struct tessera *t, char *age, void *);
#endif
