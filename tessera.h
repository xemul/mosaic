#ifndef __MOSAIC_TESSERA_H__
#define __MOSAIC_TESSERA_H__
struct tessera;
struct mosaic_state;

struct tess_desc {
	char *td_name;
	/*
	 * Add/Del callbacks from cldline
	 */
	int (*add)(struct tessera *t, int argc, char **argv);
	void (*del)(struct tessera *t);
	/*
	 * Parse/Save callback from/for config
	 * Last call to parse is with NULL/NULL and is to
	 * verify that we've read everything from config.
	 */
	int (*parse)(struct tessera *t, char *name, char *value);
	void (*save)(struct tessera *t, FILE *);
	/*
	 * Show any information about tessera on the screen
	 */
	void (*show)(struct tessera *t);

	int (*mount)(struct tessera *t, int age, char *path, char *options);
	int (*grow)(struct tessera *t, int old_age, int new_age);
};

int do_mount_tessera(struct tessera *t, int age, char *at, char *options);
struct tess_desc *tess_desc_by_type(char *type);
struct tessera *find_tessera(struct mosaic_state *ms, char *name);

/*
 * For UAPI
 */

int mosaic_iterate_tesserae(int (*cb)(struct tessera *, void *), void *x);
struct tessera *mosaic_find_tessera(char *name);
int mosaic_add_tessera(char *type, char *name, int n_opts, char **opts);
int mosaic_del_tessera(struct tessera *t);
int mosaic_mount_tessera(struct tessera *t, int age, char *at, char *options);
int mosaic_grow_tessera(struct tessera *t, int age, int from_age);
#endif
