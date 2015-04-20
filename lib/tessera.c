#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

int mosaic_iterate_tesserae(int (*cb)(struct tessera *, void *), void *x)
{
	struct tessera *t;
	int ret = 0;

	list_for_each_entry(t, &ms->tesserae, sl)
		if ((ret = cb(t, x)) != 0)
			break;

	return ret;
}

extern struct tess_desc tess_desc_overlay;

static struct tess_desc tess_desc_btrfs = {
	.td_name = "btrfs",
};

static struct tess_desc tess_desc_dmthin = {
	.td_name = "dmthin",
};

struct tess_desc *tess_desc_by_type(char *type)
{
	if (!strcmp(type, "overlay"))
		return &tess_desc_overlay;
	if (!strcmp(type, "btrfs"))
		return &tess_desc_btrfs;
	if (!strcmp(type, "dm_thin"))
		return &tess_desc_dmthin;

	return NULL;
}

struct tessera *find_tessera(struct mosaic_state *ms, char *name)
{
	struct tessera *t;

	list_for_each_entry(t, &ms->tesserae, sl)
		if (!strcmp(t->t_name, name))
			return t;

	return NULL;
}

struct tessera *mosaic_find_tessera(char *name)
{
	return find_tessera(ms, name);
}

int mosaic_mount_tessera(struct tessera *t, int age, char *path, char *options)
{
	struct stat buf;

	if (stat(path, &buf))
		return -1;

	if (!S_ISDIR(buf.st_mode))
		return -1;

	return do_mount_tessera(t, age, path, options);
}

int do_mount_tessera(struct tessera *t, int age, char *path, char *options)
{
	if (!t->t_desc->mount)
		return -1;

	return t->t_desc->mount(t, age, path, options);
}

int mosaic_grow_tessera(struct tessera *t, int new_age, int base_age)
{
	if (!t->t_desc->grow)
		return -1;

	return t->t_desc->grow(t, base_age, new_age);
}

int mosaic_add_tessera(char *type, char *name, int n_opts, char **opts)
{
	struct tess_desc *td;
	struct tessera *t;

	td = tess_desc_by_type(type);
	if (!td || !td->add)
		return -1;

	t = malloc(sizeof(*t));
	t->t_name = strdup(name);
	t->t_desc = td;

	if (td->add(t, n_opts, opts)) {
		free(name);
		free(t);
		return -1;
	}

	list_add_tail(&t->sl, &ms->tesserae);

	return config_update();
}

int mosaic_del_tessera(struct tessera *t)
{   
	/*
	 * FIXME -- what to do with on-disk layout?
	 */

	list_del(&t->sl);
	if (t->t_desc->del)
		t->t_desc->del(t);

	free(t->t_name);
	free(t);

	return config_update();
}
