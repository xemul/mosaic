#include <sys/mount.h>
#include <stdio.h>
#include <string.h>
#include "mosaic.h"

static int open_fsimg(struct mosaic *m, int flags)
{
	if (!m->default_fs)
		m->default_fs = strdup("ext4");

	return 0;
}

static int mount_fsimg(struct mosaic *m, const char *path, int mount_flags)
{
	return mount(m->m_loc, path, NULL, MS_BIND | mount_flags, NULL);
}

const struct mosaic_ops mosaic_fsimg = {
	.open = open_fsimg,
	.mount = mount_fsimg,
};
