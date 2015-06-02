#ifndef __MOSAIC_TESSERA_H__
#define __MOSAIC_TESSERA_H__
struct tessera;
struct mosaic_state;

struct tess_desc {
	char *td_name;
	/*
	 * Add/Del callbacks from cldline
	 */
	int (*add)(struct tessera *t, int n_opts, char **opts);
	void (*del)(struct tessera *t, char *age);
	/*
	 * Parse/Save callback from/for config
	 * Last call to parse is with NULL/NULL and is to
	 * verify that we've read everything from config.
	 */
	int (*parse)(struct tessera *t, char *name, char *value);
	void (*save)(struct tessera *t, FILE *);
	/*
	 * Show any information about tessera/age on the screen.
	 */
	void (*show_tess)(struct tessera *t, int off);
	void (*show_age)(struct tessera *t, char *age, int off);

	int (*mount)(struct tessera *t, char *age, char *path, char *options);
	int (*grow)(struct tessera *t, char *old_age, char *new_age);
	int (*iter_ages)(struct tessera *, int (*cb)(struct tessera *, char *, void *), void *);
};

int do_mount_tessera(struct tessera *t, char *age, char *at, char *options);
struct tess_desc *tess_desc_by_type(char *type);
struct tessera *find_tessera(struct mosaic_state *ms, char *name);
#endif
