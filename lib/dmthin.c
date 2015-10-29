#include <stdio.h>

#include "mosaic.h"
#include "volume.h"
#include "thin-internal.h"
#include "log.h"
#include "util.h"

/* ID of a thin volume with mosaic metadata part */
#define THIN_MOSAIC_SUBVOL	(0)

static int init_thin(struct mosaic *m)
{
	/* TODO: allocate private data */
	return 0;
}

static void release_thin(struct mosaic *m)
{
	/* TODO: release private data */
}

static int open_thin(struct mosaic *m, int open_flags)
{
	if (!m->default_fs)
		return -1;

	return 0;
}

static int mount_thin_mosaic(struct mosaic *m, const char *path, int mount_flags)
{
	/* TODO: mount mosaic subvolume (THIN_MOSAIC_SUBVOL) */
	return -1;
}

static int new_thin_subvol(struct mosaic *m, const char *name,
		unsigned long size_in_blocks, int make_flags)
{
	unsigned id;
	char *argv[8], creat_cmd[64];

	if (thin_id_new(m->name, name, &id) != 0) {
		loge("Name [%s] already exists\n", name);
		return -1;
	}

	argv[0] = "dmsetup";
	argv[1] = "message";
	argv[2] = m->m_loc;
	argv[3] = "0";
	snprintf(creat_cmd, sizeof(creat_cmd), "create_thin %u", id);
	argv[4] = creat_cmd;
	argv[5] = NULL;

	if (run_prg(argv)) {
		thin_id_del(m->name, name);
		return -1;
	}

	return 0;
}

static int clone_thin_subvol(struct mosaic *m, struct volume *from, const char *name, int clone_flags)
{
	/*
	 * TODO: generate new ID, save name:ID map, call dmsetup 'suspend',
	 *       create_snap ID from.ID, 'resume'
	 */
	return -1;
}

static int drop_thin_subvol(struct mosaic *m, struct volume *v, int drop_flags)
{
	unsigned id;
	char *argv[8], drop_cmd[64];

	if (thin_id_get(m->name, v->t_name, &id) != 1) {
		loge("Name [%s] doesn't exist\n", v->t_name);
		return -1;
	}

	argv[0] = "dmsetup";
	argv[1] = "message";
	argv[2] = m->m_loc;
	argv[3] = "0";
	snprintf(drop_cmd, sizeof(drop_cmd), "delete %u", id);
	argv[4] = drop_cmd;
	argv[5] = NULL;

	if (run_prg(argv))
		return -1;

	thin_id_del(m->name, v->t_name);
	return 0;
}

static int resize_thin_subvol(struct mosaic *m, struct volume *v, unsigned long size_in_blocks, int resize_flags)
{
	/* FIXME: implement */
	return -1;
}

static int open_thin_subvol(struct mosaic *m, struct volume *v, int open_flags)
{
	/* TODO: Allocate private data, read name:ID mapping */
	return -1;
}

static int close_thin_subvol(struct mosaic *m, struct volume *v)
{
	/* TODO: release private data */
	return 0;
}

static int have_thin_subvol(struct mosaic *m, const char *name, int flags)
{
	/* TODO: check mapping for name */
	return 0;
}

static int attach_thin_subvol(struct mosaic *m, struct volume *v, char *dev, int len, int flags)
{
	/* TODO: get ID from v, call dmsetup 'create thin ...' */
	return -1;
}

static int detach_thin_subvol(struct mosaic *m, struct volume *v)
{
	/* TODO: call dmsetup remove */
	return -1;
}

static int get_thin_subvol_size(struct mosaic *m, struct volume *v, unsigned long *size_in_blocks)
{
	/* FIXME: implement */
	return -1;
}

const struct mosaic_ops mosaic_thin = {
	.name = "thin",

	.init = init_thin,
	.release = release_thin,

	.open = open_thin,
	.mount = mount_thin_mosaic,

	.new_volume = new_thin_subvol,
	.open_volume = open_thin_subvol,
	.close_volume = close_thin_subvol,
	.have_volume = have_thin_subvol,
	.clone_volume = clone_thin_subvol,
	.drop_volume = drop_thin_subvol,
	.resize_volume = resize_thin_subvol,

	.attach_volume = attach_thin_subvol,
	.detach_volume = detach_thin_subvol,

	.get_volume_size = get_thin_subvol_size,
};
