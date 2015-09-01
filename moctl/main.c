#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uapi/mosaic.h"

static inline int argis(char *arg, char *is)
{
	while (1) {
		if (*is == '\0')
			return 1;
		if (*arg == '\0')
			return 0;
		if (*arg != *is)
			return 0;

		arg++;
		is++;
	}
}

static int parse_mount_flags(char *flags)
{
	return -1;
}

static int do_mosaic_mount(mosaic_t m, int argc, char **argv)
{
	char *path;
	int flags = 0;

	if (argc < 1)
		return 1;

	path = argv[0];
	if (argc >= 2) {
		flags = parse_mount_flags(argv[1]);
		if (flags == -1)
			return 1;
	}

	if (mosaic_mount(m, path, flags) < 0) {
		perror("Can't mount mosaic");
		return 1;
	}

	return 0;
}

static int do_mosaic(int argc, char **argv)
{
	mosaic_t mos;

	if (argc < 2) {
		printf("Usage: moctl mosaic <name> [mount] ...\n");
		return 1;
	}

	mos = mosaic_open(argv[0], 0);
	if (!mos) {
		printf("Error opening mosaic %s\n", argv[0]);
		return 1;
	}

	if (argis(argv[1], "mount"))
		return do_mosaic_mount(mos, argc - 2, argv + 2);

	return 1;
}

static int do_tessera(int argc, char **argv)
{
	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: moctl [mosaic|tessera] ...\n");
		return 1;
	}

	if (argis(argv[1], "mosaic"))
		return do_mosaic(argc - 2, argv + 2);
	if (argis(argv[1], "tessera"))
		return do_tessera(argc - 2, argv + 2);

	printf("Unknown object %s\n", argv[1]);
	return 1;
}
