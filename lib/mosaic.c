#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mosaic.h"
#include "log.h"
#include "uapi/mosaic.h"

const struct mosaic_ops *mosaic_find_ops(char *type)
{
	if (!strcmp(type, "fsimg"))
		return &mosaic_fsimg;
	if (!strcmp(type, "btrfs"))
		return &mosaic_btrfs;

	return NULL;
}

mosaic_t mosaic_open(const char *cfg, int open_flags)
{
	struct mosaic *m;

	if (open_flags)
		return NULL;

	m = malloc(sizeof(*m));

	if (mosaic_parse_config(cfg, m)) {
		free(m);
		return NULL;
	}

	if (m->m_ops->open(m, open_flags)) {
		free(m);
		return NULL;
	}

	return m;
}

void mosaic_close(mosaic_t m)
{
	if (m->m_ops->release)
		m->m_ops->release(m);
	if (m->default_fs)
		free(m->default_fs);
	free(m);
}

int mosaic_mount(mosaic_t m, char *path, int mount_flags)
{
	return m->m_ops->mount(m, path, mount_flags);
}
