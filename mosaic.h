#ifndef __MOSAIC_H__
#define __MOSAIC_H__

#include "list.h"

struct tess_desc;

struct tessera {
	char *t_name;
	struct tess_desc *t_desc;
	struct list_head sl;
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

#define AGE_LAST	(-1)

struct mosaic {
	char *m_name;
	struct list_head elements; /* ties element.ml */
	struct list_head sl;
};

struct mosaic_state {
	struct list_head tesserae; /* ties tessera.sl */
	struct list_head mosaics;  /* ties mosaic.sl */
};

extern struct mosaic_state *ms;

int list_mosaics(void);
int list_tesserae(void);
int add_mosaic(int argc, char **argv);
int del_mosaic(int argc, char **argv);
int add_tessera(int argc, char **argv);
int del_tessera(int argc, char **argv);
#endif
