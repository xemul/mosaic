#ifndef __MOSAIC_STATUS_H__
#define __MOSAIC_STATUS_H__
struct mosaic;
void st_set_mounted(struct mosaic *, char *path);
void st_show_mounted(struct mosaic *);
#endif
