#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "uapi/mosaic.h"
#include "moctl.h"
#include "mosaic.h"

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

	if (argc < 3) {
		printf("Usage: moctl <name> mount <tessera>|- <path> <flags>\n");
		return 1;
	}

	if (strcmp(argv[0], "-")) {
		t = mosaic_open_tess(m, argv[0], 0);
		if (!t) {
			perror("Can't open tessera");
			return 1;
		}
	}

	path = argv[1];
	flags = parse_mount_flags(argv[2]);

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

static int do_mosaic_umount(mosaic_t m, int argc, char **argv)
{
	char *path;
	tessera_t t = NULL;

	if (argc < 2) {
		printf("Usage: moctl <name> umount <tessera>|- <path>\n");
		return 1;
	}

	if (strcmp(argv[0], "-")) {
		t = mosaic_open_tess(m, argv[0], 0);
		if (!t) {
			perror("Can't open tessera");
			return 1;
		}
	}

	path = argv[1];

	if (t) {
		if (mosaic_umount_tess(t, path, 0)) {
			perror("Can't mount mosaic");
			return 1;
		}
	} else {
		if (umount(path) < 0) {
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

static int do_mosaic_drop_tess(mosaic_t m, int argc, char **argv)
{
	tessera_t t;

	if (argc < 1) {
		printf("Usage: moctl <mosaic> drop <tessera>\n");
		return 1;
	}

	t = mosaic_open_tess(m, argv[0], 0);
	if (!t) {
		printf("No tessera %s\n", argv[0]);
		return 1;
	}

	if (mosaic_drop_tess(t, 0) < 0) {
		printf("Can't drop %s\n", argv[0]);
		mosaic_close_tess(t);
		return 1;
	}

	return 0;
}

static int do_mosaic_create(char *name, int argc, char **argv)
{
	int ret;
	mosaic_t m;

	m = malloc(sizeof(*m));
	memset(m, 0, sizeof(*m));

	if (mosaic_parse_config(name, m))
		ret = -1;
	else if (!strcmp(m->m_ops->name, "fsimg"))
		ret = create_fsimg(m, argc, argv);
	else if (!strcmp(m->m_ops->name, "btrfs"))
		ret = create_btrfs(m, argc, argv);
	else if (!strcmp(m->m_ops->name, "plain"))
		ret = create_plain(m, argc, argv);
	else {
		printf("Unknown mosaic type %s\n", name);
		ret = -1;
	}

	return ret ? 1 : 0;
}

static int do_mosaic(char *name, int argc, char **argv)
{
	mosaic_t mos;

	if (argc < 1) {
		printf("Usage: moctl <name> [mount|new|clone|drop] ...\n");
		return 1;
	}

	if (argis(argv[0], "create"))
		return do_mosaic_create(name, argc - 1, argv + 1);

	mos = mosaic_open(name, 0);
	if (!mos) {
		printf("Error opening mosaic %s\n", name);
		return 1;
	}

	if (argis(argv[0], "mount"))
		return do_mosaic_mount(mos, argc - 1, argv + 1);
	if (argis(argv[0], "umount"))
		return do_mosaic_umount(mos, argc - 1, argv + 1);
	if (argis(argv[0], "new"))
		return do_mosaic_new_tess(mos, argc - 1, argv + 1);
	if (argis(argv[0], "clone"))
		return do_mosaic_clone_tess(mos, argc - 1, argv + 1);
	if (argis(argv[0], "drop"))
		return do_mosaic_drop_tess(mos, argc - 1, argv + 1);

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
