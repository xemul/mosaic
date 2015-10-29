#include <mosaic.h>

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
	/* TODO: generate new ID, save name:ID map, call dmsetup 'create_thin ID' */
	return -1;
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
	/* TODO: call dmsetup 'delete ID', remove name:ID mapping */
	return -1;
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
