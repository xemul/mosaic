#ifndef __DM_THIN_ID_H__
#define __DM_THIN_ID_H__
#include <stdbool.h>
int thin_get_id(char *dev, char *tess, char *age, bool new);

struct thin_map {
	char *tess;
	char *age;
	int vol_id;
};

int thin_walk_ids(char *dev, int (*cb)(struct thin_map *, void *), void *);
int thin_del_ids(char *dev, int (*cb)(struct thin_map *, void *), void *x);
#endif
