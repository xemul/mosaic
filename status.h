#ifndef __MOSAIC_STATUS_H__
#define __MOSAIC_STATUS_H__
#include <stdbool.h>

struct mosaic;
void st_set_mounted(struct mosaic *, char *path);
void st_show_mounted(struct mosaic *);

int st_for_each_mounted(struct mosaic *m, bool mod, int (*cb)(struct mosaic *, char *, void *), void *x);

#define ST_OK	0
#define ST_FAIL	-1
#define ST_DROP	1  /* drop this entry from status file */
#endif
