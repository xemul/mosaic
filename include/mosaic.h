#ifndef __MOSAIC_H__
#define __MOSAIC_H__

#include "list.h"

struct tess_desc;

struct tessera {
	char *t_name;
	struct tess_desc *t_desc;
	struct list_head sl;
	void *priv;
};

struct element {
	union {
		struct tessera *t;
		char *e_name; /* parse-time only */
	};
	int e_age;
	char *e_at;
	char *e_options;
	struct list_head ml;
};

struct mosaic {
	char *m_name;
	struct list_head elements; /* ties element.ml */
	struct list_head sl;
};

struct mosaic_state {
	struct list_head tesserae; /* ties tessera.sl */
	struct list_head mosaics;  /* ties mosaic.sl */
};

/*
 * This is for UAPI
 */

int mosaic_iterate(int (*cb)(struct mosaic *, void *), void *);
struct mosaic *mosaic_find_by_name(char *name);
int mosaic_iterate_elements(struct mosaic *, int (*cb)(struct mosaic *, struct element *, void *), void *);

struct mosaic *mosaic_new(char *name);
int mosaic_set_element(struct mosaic *m, char *name, int age, char *at, char *opt);
int mosaic_del_element(struct mosaic *m, char *name);
int mosaic_add(struct mosaic *m);
int mosaic_update(struct mosaic *m);
int mosaic_del(struct mosaic *m);

int mosaic_mount(struct mosaic *m, char *mountpoint, char *options);
int mosaic_umount(struct mosaic *m, char *mountpoint);
#endif
