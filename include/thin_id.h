#ifndef __DM_THIN_ID_H__
#define __DM_THIN_ID_H__
#include <stdbool.h>
int thin_get_id(char *dev, char *tess, int age, bool new);
int thin_walk_ids(char *dev, int (*cb)(char *t, int age, int vol, void *), void *);
#endif
