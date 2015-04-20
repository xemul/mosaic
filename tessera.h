#ifndef __MOSAIC_TESSERA_H__
#define __MOSAIC_TESSERA_H__
struct tessera;

struct tess_desc {
	char *td_name;
	int (*add)(struct tess_desc *me, char *name, int argc, char **argv);
	void (*del)(struct tess_desc *me, struct tessera *t);
	int (*parse)(struct tessera *t, char *name, char *value);
	void (*save)(struct tessera *t, FILE *);
	void (*show)(struct tessera *t);
	int (*mount)(struct tessera *t, int age, char *path, char *options);
};

int do_tessera(int argc, char **argv);

struct tess_desc *tess_desc_by_type(char *type);
struct tessera *find_tessera(struct mosaic_state *ms, char *name);
#endif
