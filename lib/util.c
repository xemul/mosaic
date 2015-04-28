#include <sys/mount.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int parse_mount_opts(char *opt, int *m_flags)
{
	char *aux;

	*m_flags = 0;

	while (opt) {
		aux = strchr(opt, ',');
		if (aux) {
			*aux = '\0';
			aux++;
		}

		if (!strcmp(opt, "ro"))
			*m_flags |= MS_RDONLY;
		else if (!strcmp(opt, "rw"))
			;
		else
			return -1;

		opt = aux;
	}

	return 0;
}

#define SECTOR_SHIFT	(9)

unsigned long parse_blk_size(char *str)
{
	char *end;
	unsigned long ret;

	ret = strtoul(str, &end, 10);
	switch (*end) {
	case 'k':
	case 'K':
		ret <<= 10;
		break;
	case 'm':
	case 'M':
		ret <<= 20;
		break;
	case 'g':
	case 'G':
		ret <<= 30;
		break;
	case 's':
		/* sectors */
		ret <<= SECTOR_SHIFT;
		break;
	default:
		return 0;
	}

	return ret;
}
