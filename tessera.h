#ifndef __MOSAIC_TESSERA_H__
#define __MOSAIC_TESSERA_H__
struct tessera;

struct tess_desc {
	char *td_name;
	int (*add)(struct tess_desc *me, char *name, int argc, char **argv);
	int (*del)(struct tess_desc *me, struct tessera *t);
};

struct tess_desc *tess_desc_by_type(char *type);
#endif
