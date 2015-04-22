#include <sys/mount.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include "mosaic.h"
#include "tessera.h"
#include "overlay.h"

#ifndef OVERLAYFS_SUPER_MAGIC
#define OVERLAYFS_SUPER_MAGIC 0x794c7630
#endif

struct overlay_tessera {
	char *ovl_location;
};

struct tess_desc tess_desc_overlay;

static int add_overlay(struct tessera *t, int n_opts, char **opts)
{
	struct overlay_tessera *ot;

	if (n_opts < 1) {
		printf("Usage: moctl tessera add <name> overlay <location>\n");
		return -1;
	}

	if (access(opts[0], F_OK) < 0) {
		if (mkdir(opts[0], 0600) < 0) {
			perror("Can't make dir for base");
			return -1;
		}
	}

	t->priv = ot = malloc(sizeof(*ot));
	ot->ovl_location = strdup(opts[0]);

	return 0;
}

static void umount_deltas(struct tessera *t);

static void del_overlay(struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;

	umount_deltas(t);

	free(ot->ovl_location);
	free(ot);
}

static int parse_overlay(struct tessera *t, char *key, char *val)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot) {
		ot = malloc(sizeof(*ot));
		ot->ovl_location = NULL;
		t->priv = ot;
	}

	if (!key)
		return ot->ovl_location ? 0 : -1;

	if (!strcmp(key, "location")) {
		ot->ovl_location = val;
		return 0;
	}

	return -1;
}

static void save_overlay(struct tessera *t, FILE *f)
{
	struct overlay_tessera *ot = t->priv;

	fprintf(f, "    location: %s\n", ot->ovl_location);
}

static int iterate_ages(struct tessera *t, int (*cb)(struct tessera *t, char *age_d, void *), void *x)
{
	struct overlay_tessera *ot = t->priv;
	DIR *d;
	struct dirent *de;
	int ret = 0;

	d = opendir(ot->ovl_location);
	if (!d)
		return -1;

	while ((de = readdir(d)) != NULL) {
		if (de->d_name[0] == '.')
			continue;

		if (!strcmp(de->d_name, "base"))
			continue;

		if (strncmp(de->d_name, "age-", 4))
			/* FIXME -- what? */
			continue;

		ret = cb(t, de->d_name, x);
		if (ret)
			break;
	}

	closedir(d);

	return ret;
}

static int show_age(struct tessera *t, char *age_d, void *x)
{
	int *p = x;

	if (!*p) {
		*p = 1;
		printf("ages:\n");
	}

	printf("  - %s\n", age_d + 4);
	return 0;
}

static void show_overlay(struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;
	int p = 0;

	printf("location: %s\n", ot->ovl_location);
	iterate_ages(t, show_age, &p);
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

	sprintf(aux, "upperdir=%s/age-%d/data,workdir=%s/age-%d/work,lowerdir=%s/",
			ot->ovl_location, age, ot->ovl_location, age, l_path);
	sprintf(l_path + l_off, "/age-%d/root", age);

	if (mount("none", l_path, "overlay", 0, aux)) {
		perror("Can't mount overlay");
		return -1;
	}

	return 0;
}

struct umount_ctx {
	char *path;
	int p_off;
};

static int umount_overlay_delta(struct tessera *t, char *age_d, void *x)
{
	struct umount_ctx *uc = x;

	sprintf(uc->path + uc->p_off, "%s/root", age_d);
	umount(uc->path);

	return 0;
}

static void umount_deltas(struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;
	char path[PATH_MAX];
	struct umount_ctx uc;

	uc.path = path;
	uc.p_off = sprintf(path, "%s/", ot->ovl_location);
	iterate_ages(t, umount_overlay_delta, &uc);

	sprintf(path + uc.p_off, "base");
	umount(path);
}

static int mount_overlay(struct tessera *t, int age, char *path, char *options)
{
	struct overlay_tessera *ot = t->priv;
	char l_path[PATH_MAX];
	int plen;

	if (options) {
		printf("Mount options are not yet supported\n");
		return -1;
	}

	plen = sprintf(l_path, "%s", ot->ovl_location);
	if (mount_overlay_delta(t, age, l_path, plen))
		return -1;

	if (mount(l_path, path, NULL, MS_BIND, NULL) < 0) {
		perror("Can't bind mount overlay");
		return -1;
	}

	return 0;
}

static int grow_overlay(struct tessera *t, int base_age, int new_age)
{
	struct overlay_tessera *ot = t->priv;
	char path[PATH_MAX], aux[32];
	int plen, i;
	char *subs[] = { "/data", "/work", "/root", ".parent", NULL, };

	plen = sprintf(path, "%s/age-%d", ot->ovl_location, new_age);
	if (mkdir(path, 0600)) {
		perror("Can't make snapshot");
		return -1;
	}

	for (i = 0; subs[i] != NULL; i++) {
		int ret;

		sprintf(path + plen, "/%s", subs[i] + 1);
		if (subs[i][0] == '/')
			ret = mkdir(path, 0600);
		else {
			sprintf(aux, "%d", base_age);
			ret = symlink(aux, path);
		}

		if (ret)
			goto cleanup;
	}

	return 0;

cleanup:
	for (i = 0; subs[i] != NULL; i++) {
		sprintf(path + plen, "/%s", subs[i] + 1);
		if (subs[i][0] == '/')
			rmdir(path);
		else
			unlink(path);
	}

	*(path + plen) = '\0';
	rmdir(path);
	return -1;
}

struct tess_desc tess_desc_overlay = {
	.td_name = "overlay",
	.add = add_overlay,
	.del = del_overlay,
	.parse = parse_overlay,
	.save = save_overlay,
	.show = show_overlay,
	.mount = mount_overlay,
	.grow = grow_overlay,
};
