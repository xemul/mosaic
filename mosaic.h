#ifndef __MOSAIC_H__
#define __MOSAIC_H__

#include "list.h"

enum tess_type {
	TESSERA_UNKNOWN,
	TESSERA_OVERLAY,
	TESSERA_BTRFS,
	TESSERA_DMTHIN,
};

struct tessera {
	char *t_name;
	enum tess_type t_type;
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

#endif
