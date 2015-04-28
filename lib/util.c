#include <sys/mount.h>
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
