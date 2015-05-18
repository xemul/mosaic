#include <sys/stat.h>
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"
#include "status.h"

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
extern struct tess_desc tess_desc_thin;
extern struct tess_desc tess_desc_plain;
extern struct tess_desc tess_desc_ephemeral;

static struct tess_desc tess_desc_btrfs = {
	.td_name = "btrfs",
};

static struct tess_desc *t_types[] = {
	&tess_desc_overlay,
	&tess_desc_thin,
	&tess_desc_plain,
	&tess_desc_ephemeral,
	&tess_desc_btrfs,
	NULL,
};

struct tess_desc *tess_desc_by_type(char *type)
{
	int i;

	for (i = 0; t_types[i] != NULL; i++)
		if (!strcmp(t_types[i]->td_name, type))
			return t_types[i];

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
	int ret;

	if (stat(path, &buf))
		return -1;

	if (!S_ISDIR(buf.st_mode))
		return -1;

	ret = do_mount_tessera(t, age, path, options);
	if (!ret)
		st_set_mounted_t(t, age, path);

	return ret;
}

static int do_umount_tessera(struct tessera *t, int age, char *path)
{
	return umount(path);
}

int mosaic_umount_tessera(struct tessera *t, int age, char *path)
{
	return st_umount_t(t, age, path, do_umount_tessera);
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
		free(t->t_name);
		free(t);
		return -1;
	}

	list_add_tail(&t->sl, &ms->tesserae);

	return config_update();
}

int mosaic_iterate_ages_t(struct tessera *t, int (*cb)(struct tessera *, int age, void *), void *x)
{
	if (!t->t_desc->iter_ages)
		/* Special case -- only base tessera available */
		return cb(t, 0, x);

	return t->t_desc->iter_ages(t, cb, x);
}

int mosaic_del_tessera(struct tessera *t, int age)
{
	struct tess_desc *td = t->t_desc;

	/*
	 * FIXME -- what to do with on-disk layout?
	 * FIXME -- this ages iteration is not nice, would
	 * (probably) be better to keep mounted state for
	 * all ages in one status file.
	 * FIXME -- can be mounted in mosaic
	 */

	if (age == -1) {
		if (mosaic_iterate_ages_t(t, st_is_mounted_t, NULL))
			return -1;
	} else
		return -1; /* FIXME  implement */

	list_del(&t->sl);
	if (td->del)
		td->del(t, age);

	free(t->t_name);
	free(t);

	return config_update();
}

void mosaic_print_types_t(void)
{
	int i;

	for (i = 0; t_types[i] != NULL; i++)
		printf("%s\n", t_types[i]->td_name);
}
