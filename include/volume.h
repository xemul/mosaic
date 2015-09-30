#ifndef __MOSAIC_VOLUME_H__
#define __MOSAIC_VOLUME_H__
struct mosaic;

struct volume {
	struct mosaic *m;
	char *t_name;
};
#endif
