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
#include "status.h"
#include "util.h"
#include "log.h"
#include "config.h"

#ifndef OVERLAYFS_SUPER_MAGIC
#define OVERLAYFS_SUPER_MAGIC 0x794c7630
#endif

struct overlay_tessera {
	char *ovl_location;
};

static int add_overlay(struct tessera *t, int n_opts, char **opts)
{
	struct overlay_tessera *ot;

	if (n_opts < 1) {
		log("Usage: moctl tessera add <name> overlay <location>\n");
		return -1;
	}

	if (access(opts[0], F_OK) < 0) {
		if (mkdir(opts[0], 0600) < 0) {
			loge("Can't make dir for base");
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

static inline void print_overlay_info(FILE *f, int off, struct overlay_tessera *ot)
{
	fprintf(f, "%*slocation: %s\n", off, "", ot->ovl_location);
}

static void save_overlay(struct tessera *t, FILE *f)
{
	print_overlay_info(f, CFG_TESS_OFF, t->priv);
}

static void show_overlay(struct tessera *t, int age)
{
	if (age == -1)
		print_overlay_info(stdout, 0, t->priv);
}

static int iterate_ages(struct tessera *t, int (*cb)(struct tessera *t, int age, void *), void *x)
{
	struct overlay_tessera *ot = t->priv;
	DIR *d;
	struct dirent *de;
	int ret = 0;

	ret = cb(t, 0, x);
	if (ret)
		return ret;

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

		ret = cb(t, atoi(de->d_name + 4), x);
		if (ret)
			break;
	}

	closedir(d);

	return ret;
}

/*
 * Overlay location layout:
 *
 * location/
 *   base/     -> base delta
 *     data/   -> with contents
 *     wlock   -> write locked
 *   age-N/
 *     data/   -> for upperdir
 *     work/   -> for workdir
 *     parent  -> symlink on parent age
 *     root/   -> for mountpoint
 *     wlock   -> write locked
 */

static int mount_overlay_delta(struct tessera *t, int age, char *l_path, int l_off, int *top_ro)
{
	struct overlay_tessera *ot = t->priv;
	char aux[4096];
	struct statfs buf;

	if (age == 0) {
		l_off += sprintf(l_path + l_off, "/base");

		if (access(l_path, F_OK) < 0) {
			if (mkdir(l_path, 0600) < 0) {
				loge("Can't make dir for base");
				return -1;
			}
		}

		if (top_ro) {
			sprintf(l_path + l_off, "/wlock");
			*top_ro = (access(l_path, F_OK) == 0);
		}

		sprintf(l_path + l_off, "/data");
		if (access(l_path, F_OK) < 0) {
			if (mkdir(l_path, 0600) < 0) {
				loge("Can't make dir for data");
				return -1;
			}
		}

		return 0; /* always there */
	}

	/*
	 * Check the delta exists at all
	 */
	sprintf(l_path + l_off, "/age-%d", age);
	if (access(l_path, F_OK) < 0)
		return -1;

	/*
	 * Check that mountpoint is already set up
	 */
	sprintf(l_path + l_off, "/age-%d/root", age);
	if (statfs(l_path, &buf) < 0)
		return -1;

	if (buf.f_type == OVERLAYFS_SUPER_MAGIC)
		return 0;

	/*
	 * It's not. Time to check for the lower delta and mount
	 * this one.
	 */
	sprintf(l_path + l_off, "/age-%d/parent", age);
	if (readlink(l_path, aux, sizeof(aux)) < 0) {
		loge("Can't readlink");
		return -1;
	}

	/*
	 * Check parent mount and ask for it into l_path
	 */
	if (mount_overlay_delta(t, atoi(aux), l_path, l_off, NULL))
		return -1;

	sprintf(aux, "upperdir=%s/age-%d/data,workdir=%s/age-%d/work,lowerdir=%s/",
			ot->ovl_location, age, ot->ovl_location, age, l_path);

	if (top_ro) {
		sprintf(l_path + l_off, "/age-%d/wlock", age);
		*top_ro = (access(l_path, F_OK) == 0);
	}

	sprintf(l_path + l_off, "/age-%d/root", age);

	if (mount("none", l_path, "overlay", 0, aux)) {
		loge("Can't mount overlay");
		return -1;
	}

	return 0;
}

struct umount_ctx {
	char *path;
	int p_off;
};

static int umount_overlay_delta(struct tessera *t, int age, void *x)
{
	struct umount_ctx *uc = x;

	if (age == 0)
		return 0;

	sprintf(uc->path + uc->p_off, "age-%d/root", age);
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
}

static int mount_overlay(struct tessera *t, int age, char *path, char *options)
{
	struct overlay_tessera *ot = t->priv;
	char l_path[PATH_MAX];
	int plen, m_flags, ro;

	if (parse_mount_opts(options, &m_flags))
		return -1;

	plen = sprintf(l_path, "%s", ot->ovl_location);
	if (mount_overlay_delta(t, age, l_path, plen, &ro))
		return -1;

	if (ro && !(m_flags & MS_RDONLY))
		return -1;

	if (mount(l_path, path, NULL, MS_BIND | m_flags, NULL) < 0) {
		loge("Can't bind mount overlay");
		return -1;
	}

	if (m_flags & MS_RDONLY)
		mount(NULL, path, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY, NULL);

	return 0;
}

static int grow_overlay(struct tessera *t, int base_age, int new_age)
{
	struct overlay_tessera *ot = t->priv;
	char path[PATH_MAX], aux[32];
	int plen, i;
	char *subs[] = { "/data", "/work", "/root", ".parent", NULL, };

	/*
	 * FIXME -- this checks only pure mount, in-mosaic
	 * mount should also block this
	 */

	if (st_is_mounted_t(t, base_age, NULL))
		return -1;

	plen = sprintf(path, "%s/age-%d", ot->ovl_location, new_age);
	if (mkdir(path, 0600)) {
		loge("Can't make snapshot");
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

	if (base_age)
		sprintf(path, "%s/age-%d/wlock", ot->ovl_location, base_age);
	else
		sprintf(path, "%s/base/wlock", ot->ovl_location);

	if (mknod(path, S_IFREG | 0600, 0))
		goto cleanup;

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
	.iter_ages = iterate_ages,
};
