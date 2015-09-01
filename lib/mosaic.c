#include <stdlib.h>
#include <stdio.h>

#include "mosaic.h"
#include "log.h"
#include "uapi/mosaic.h"

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

	return m;
}

void mosaic_close(mosaic_t m)
{
	free(m);
}

int mosaic_mount(mosaic_t m, char *path, int mount_flags)
{
	return -1;
}

void mosaic_set_tessera_fs(mosaic_t m, char *fsname)
{
}
