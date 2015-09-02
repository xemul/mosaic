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
	return 0;
}

static int do_mosaic_mount(mosaic_t m, int argc, char **argv)
{
	char *path;
	tessera_t t = NULL;
	int flags;

	if (argc < 2) {
		printf("Usage: moctl <name> mount [<tessera>] <path> <flags>\n");
		return 1;
	}

	if (argc >= 3) {
		t = mosaic_open_tess(m, argv[0], 0);
		if (!t) {
			perror("Can't open tessera");
			return 1;
		}

		argc--;
		argv++;
	}

	path = argv[0];
	flags = parse_mount_flags(argv[1]);

	if (t) {
		if (mosaic_mount_tess(t, path, flags)) {
			perror("Can't mount mosaic");
			return 1;
		}
	} else {
		if (mosaic_mount(m, path, flags) < 0) {
			perror("Can't mount mosaic");
			return 1;
		}
	}

	return 0;
}

static int parse_size(char *sz)
{
	char *end;
	unsigned long ret;

	ret = strtol(sz, &end, 10);
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
	case 't':
	case 'T':
		ret <<= 40;
		break;
	}

	ret >>= 9;
	return ret;
}

static int do_mosaic_new_tess(mosaic_t m, int argc, char **argv)
{
	int ret;
	unsigned long size;

	if (argc < 3) {
		printf("Usage: moctl <name> new fs|disk <name> <size>\n");
		return 1;
	}

	size = parse_size(argv[2]);

	if (argis(argv[0], "disk"))
		ret = mosaic_make_tess(m, argv[1], size, 0);
	else if (argis(argv[0], "fs"))
		ret = mosaic_make_tess_fs(m, argv[1], size, 0);
	else
		return 1;

	return ret ? 1 : 0;
}

static int do_mosaic_clone_tess(mosaic_t m, int argc, char **argv)
{
	int ret;
	tessera_t old;

	if (argc < 2) {
		printf("Usage: moctl <name> clone <old> <new>\n");
		return 1;
	}

	old = mosaic_open_tess(m, argv[0], 0);
	if (!old) {
		printf("No tessera %s\n", argv[0]);
		return 1;
	}

	ret = mosaic_clone_tess(old, argv[1], 0);
	mosaic_close_tess(old);

	if (ret < 0) {
		printf("Can't clone %s\n", argv[0]);
		return 1;
	}

	return 0;
}

static int do_mosaic(char *name, int argc, char **argv)
{
	mosaic_t mos;

	if (argc < 1) {
		printf("Usage: moctl <name> [mount|new|clone] ...\n");
		return 1;
	}

	mos = mosaic_open(name, 0);
	if (!mos) {
		printf("Error opening mosaic %s\n", argv[0]);
		return 1;
	}

	if (argis(argv[0], "mount"))
		return do_mosaic_mount(mos, argc - 1, argv + 1);
	if (argis(argv[0], "new"))
		return do_mosaic_new_tess(mos, argc - 1, argv + 1);
	if (argis(argv[0], "clone"))
		return do_mosaic_clone_tess(mos, argc - 1, argv + 1);

	return 1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: moctl <mosaic_name> <action> [<arguments>]\n");
		return 1;
	}

	return do_mosaic(argv[1], argc - 2, argv + 2);
}
