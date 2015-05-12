#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "mosaic.h"
#include "tessera.h"
#include "thin_id.h"

struct thin_tessera {
	char *thin_dev;
	char *thin_fs;
	unsigned long thin_age_size;
};

static int thin_init_base(char *t_name, char *dev, char *fs, unsigned long sz)
{
	char cmd[1024];
	int id;

	/*
	 * FIXME -- as a prototype system() is used.
	 * Would be nice to rewrite this using libdevicemapper
	 */

	id = thin_get_id(dev, t_name, 0, true);
	if (id < 0)
		return -1;

	sprintf(cmd, "dmsetup message %s 0 \"create_thin %d\"", dev, id);
	if (system(cmd))
		return -1;

	sprintf(cmd, "dmsetup create mosaic-%s-0 --table \"0 %lu thin %s %d\"",
			t_name, sz >> SECTOR_SHIFT, dev, id);
	if (system(cmd))
		return -1;

	sprintf(cmd, "mkfs.%s /dev/mapper/mosaic-%s-0\n", fs, t_name);
	if (system(cmd))
		return -1;

	return 0;
}

static int add_thin(struct tessera *t, int n_opts, char **opts)
{
	struct thin_tessera *tt;
	unsigned long sz;

	if (n_opts < 3) {
		printf("Usage: moctl tessera add <name> dm_thin <device> <fstype> <age-size>\n");
		return -1;
	}

	sz = parse_blk_size(opts[2]);
	if (!sz) {
		printf("Invalid size %s\n", opts[1]);
		return -1;
	}

	if (thin_init_base(t->t_name, opts[0], opts[1], sz))
		return -1;

	t->priv = tt = malloc(sizeof(*tt));
	tt->thin_dev = strdup(opts[0]);
	tt->thin_fs = strdup(opts[1]);
	tt->thin_age_size = sz;

	return 0;
}

static void del_thin(struct tessera *t)
{
	struct thin_tessera *tt = t->priv;
	char cmd[1024];

	sprintf(cmd, "dmsetup remove mosaic-%s-0", t->t_name);
	system(cmd);

	free(tt->thin_dev);
	free(tt->thin_fs);
	free(tt);
}

static int parse_thin(struct tessera *t, char *key, char *val)
{
	struct thin_tessera *tt = t->priv;

	if (!tt) {
		tt = malloc(sizeof(*tt));
		tt->thin_dev = NULL;
		tt->thin_age_size = 0;
		t->priv = tt;
	}

	if (!key) {
		if (!tt->thin_dev)
			return -1;
		if (!tt->thin_fs)
			return -1;
		if (!tt->thin_age_size)
			return -1;

		return 0;
	}

	if (!strcmp(key, "device")) {
		tt->thin_dev = val;
		return 0;
	}

	if (!strcmp(key, "fs")) {
		tt->thin_fs = val;
		return 0;
	}

	if (!strcmp(key, "age_size")) {
		tt->thin_age_size = parse_blk_size(val);
		free(val);
		return 0;
	}

	return -1;
}

static void save_thin(struct tessera *t, FILE *f)
{
	struct thin_tessera *tt = t->priv;

	fprintf(f, "    device: %s\n", tt->thin_dev);
	fprintf(f, "    fs: %s\n", tt->thin_fs);
	fprintf(f, "    age_size: %lu\n", tt->thin_age_size);
}

static void show_thin(struct tessera *t)
{
	struct thin_tessera *tt = t->priv;

	printf("device: %s\n", tt->thin_dev);
	printf("fs: %s\n", tt->thin_fs);
	printf("age_size: %lu\n", tt->thin_age_size);
}

static int mount_thin(struct tessera *t, int age, char *path, char *options)
{
	char dev[1024];
	struct thin_tessera *tt = t->priv;
	int m_flags;

	if (parse_mount_opts(options, &m_flags))
		return -1;

	/*
	 * FIXME -- device is added on ->add, not on config load
	 */

	sprintf(dev, "/dev/mapper/mosaic-%s-%d", t->t_name, age);
	return mount(dev, path, tt->thin_fs, m_flags, NULL);
}

static int grow_thin(struct tessera *t, int base_age, int new_age)
{
	struct thin_tessera *tt = t->priv;
	int base_id, id;
	char cmd[1024];

	base_id = thin_get_id(tt->thin_dev, t->t_name, base_age, false);
	if (base_id < 0)
		return -1;

	id = thin_get_id(tt->thin_dev, t->t_name, new_age, true);
	if (id < 0)
		return -1;

	sprintf(cmd, "dmsetup message %s 0 \"create_snap %d %d\"", tt->thin_dev, id, base_id);
	if (system(cmd))
		return -1;

	sprintf(cmd, "dmsetup create mosaic-%s-%d --table \"0 %lu thin %s %d\"",
			t->t_name, new_age, tt->thin_age_size >> SECTOR_SHIFT, tt->thin_dev, id);
	if (system(cmd))
		return -1;

	return 0;
}

struct iag {
	int (*cb)(struct tessera *t, int age, void *);
	struct tessera *t;
	void *arg;
};

static int thin_age(struct thin_map *tm, void *x)
{
	struct iag *i = x;

	if (strcmp(tm->tess, i->t->t_name))
		return 0;

	if (i->cb(i->t, tm->age, i->arg))
		return -1;

	return 0;
}

static int iterate_ages(struct tessera *t, int (*cb)(struct tessera *t, int age, void *), void *x)
{
	struct thin_tessera *tt = t->priv;
	struct iag i = { .cb = cb, .t = t, .arg = x, };

	return thin_walk_ids(tt->thin_dev, thin_age, &i);
}


struct tess_desc tess_desc_thin = {
	.td_name = "dm_thin",
	.add = add_thin,
	.del = del_thin,
	.parse = parse_thin,
	.save = save_thin,
	.show = show_thin,
	.mount = mount_thin,
	.grow = grow_thin,
	.iter_ages = iterate_ages,
};
