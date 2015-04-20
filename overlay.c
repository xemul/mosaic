#include <sys/mount.h>
#include <sys/vfs.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "overlay.h"

#ifndef OVERLAYFS_SUPER_MAGIC
#define OVERLAYFS_SUPER_MAGIC 0x794c7630
#endif

struct overlay_tessera {
	char *ovl_location;
};

static int add_overlay(struct tess_desc *me, char *name, int argc, char **argv)
{
	struct tessera *t;
	struct overlay_tessera *ot;

	if (argc < 1) {
		printf("Usage: moctl tessera add overlay [name] [location]\n");
		return 1;
	}

	ot = malloc(sizeof(*ot));
	ot->ovl_location = strdup(argv[0]);

	t = malloc(sizeof(*t));
	t->t_name = name;
	t->t_desc = me;
	t->priv = ot;

	list_add_tail(&t->sl, &ms->tesserae);

	return 0;
}

static void del_overlay(struct tess_desc *td, struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	free(ot->ovl_location);
	free(ot);
}

static int parse_overlay(struct tessera *t, char *key, char *val)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot) {
		ot = malloc(sizeof(*ot));
		t->priv = ot;
	}

	if (!strcmp(key, "location")) {
		ot->ovl_location = strdup(val);
		return 0;
	}

	return -1;
}

static void save_overlay(struct tessera *t, FILE *f)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	fprintf(f, "    location: %s\n", ot->ovl_location);
}

static void show_overlay(struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	printf("location: %s\n", ot->ovl_location);
}

/*
 * Overlay location layout:
 *
 * location/
 *   base/     -> base delta
 *   age-N/
 *     data/   -> for upperdir
 *     work/   -> for workdir
 *     parent  -> symlink on parent age
 *     root/   -> for mountpoint
 */

static int mount_overlay_delta(struct tessera *t, int age, char *l_path, int l_off)
{
	struct overlay_tessera *ot = t->priv;
	char aux[4096];
	struct statfs buf;

	if (age == 0) {
		sprintf(l_path + l_off, "/base");

		if (access(l_path, F_OK) < 0) {
			if (mkdir(l_path, 0600) < 0) {
				perror("Can't make dir for base");
				return -1;
			}
		}

		return 0; /* always there */
	}

	/*
	 * Check the delta exists at all
	 */
	sprintf(l_path + l_off, "/age-%d", age);
	if (access(l_path, F_OK) < 0) {
		printf("Age %d of %s doesn't exist\n", age, t->t_name);
		return -1;
	}

	/*
	 * Check that mountpoint is already set up
	 */
	sprintf(l_path + l_off, "/age-%d/root", age);
	if (statfs(l_path, &buf) < 0) {
		perror("Can't stat");
		return -1;
	}

	if (buf.f_type == OVERLAYFS_SUPER_MAGIC)
		return 0;

	/*
	 * It's not. Time to check for the lower delta and mount
	 * this one.
	 */
	sprintf(l_path + l_off, "/age-%d/parent", age);
	if (readlink(l_path, aux, sizeof(aux)) < 0) {
		perror("Can't readlink");
		return -1;
	}

	/*
	 * Check parent mount and ask for it into l_path
	 */
	if (mount_overlay_delta(t, atoi(aux), l_path, l_off))
		return -1;

	sprintf(aux, "upperdir=%s/data,workdir=%s/work,lowerdir=%s/",
			ot->ovl_location, ot->ovl_location, l_path);
	sprintf(l_path + l_off, "/age-%d/root", age);

	if (mount("none", l_path, "overlay", 0, aux)) {
		perror("Can't mount overlay");
		return -1;
	}

	return 0;
}

static int mount_overlay(struct tessera *t, int age, char *path, char *options)
{
	struct overlay_tessera *ot = t->priv;
	char l_path[PATH_MAX];
	int plen;

	plen = sprintf(path, "%s", ot->ovl_location);
	if (mount_overlay_delta(t, age, l_path, plen))
		return -1;

	if (mount(l_path, path, NULL, MS_BIND, NULL) < 0) {
		perror("Can't bind mount overlay");
		return -1;
	}

	return 0;
}

struct tess_desc tess_desc_overlay = {
	.td_name = "overlay",
	.add = add_overlay,
	.del = del_overlay,
	.parse = parse_overlay,
	.save = save_overlay,
	.show = show_overlay,
	.mount = mount_overlay,
};
