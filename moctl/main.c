#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include "uapi/mosaic.h"
#include "moctl.h"
#include "mosaic.h"

int usage(int ret)
{
	printf("\n"
"Usage: moctl NAME ACTION [ARGUMENT ...]\n"
"	NAME     := mosaic name (path to .mos file)\n"
"	ACTION   := mount|umount|new|clone|drop\n"
"	ARGUMENT := zero or more arguments, depending on ACTION\n");

	return ret;
}

/* This allows for abbreviated commands to be used,
 * such as 'm' instead of 'mount'.
 */
static inline int argis(const char *arg, const char *is)
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
	const char *volume, *path;
	tessera_t t = NULL;
	int flags;

	if (argc < 3) {
		printf("Usage: moctl <name> mount <tessera>|- <path> <flags>\n");
		return 1;
	}

	volume = argv[0];
	if (strcmp(volume, "-")) {
		t = mosaic_open_tess(m, volume, 0);
		if (!t) {
			perror("Can't open tessera");
			return 1;
		}
	}

	path = argv[1];
	flags = parse_mount_flags(argv[2]);

	if (t) {
		if (mosaic_mount_tess(t, path, flags)) {
			perror("Can't mount tessera");
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
	const char *volume, *path;
	tessera_t t = NULL;

	if (argc < 2) {
		printf("Usage: moctl <name> umount <tessera>|- <path>\n");
		return 1;
	}

	volume = argv[1];
	if (strcmp(volume, "-")) {
		t = mosaic_open_tess(m, volume, 0);
		if (!t) {
			perror("Can't open tessera");
			return 1;
		}
	}

	path = argv[1];
	if (t) {
		if (mosaic_umount_tess(t, path, 0)) {
			perror("Can't umount tessera");
			return 1;
		}
	} else {
		if (umount(path) < 0) {
			perror("Can't umount mosaic");
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
	const char *type, *volume;
	unsigned long size;

	if (argc < 3) {
		printf("Usage: moctl <name> new fs|disk <name> <size>\n");
		return 1;
	}

	type = argv[0];
	volume = argv[1];
	size = parse_size(argv[2]);

	if (argis(type, "disk"))
		ret = mosaic_make_tess(m, volume, size, 0);
	else if (argis(type, "fs"))
		ret = mosaic_make_tess_fs(m, volume, size, 0);
	else
		return 1;

	return ret ? 1 : 0;
}

static int do_mosaic_clone_tess(mosaic_t m, int argc, char **argv)
{
	int ret;
	const char *oldvol, *newvol;
	tessera_t old;

	if (argc < 2) {
		printf("Usage: moctl <name> clone <old> <new>\n");
		return 1;
	}

	oldvol = argv[0];
	newvol = argv[1];

	old = mosaic_open_tess(m, oldvol, 0);
	if (!old) {
		fprintf(stderr, "No tessera %s\n", oldvol);
		return 1;
	}

	ret = mosaic_clone_tess(old, newvol, 0);
	mosaic_close_tess(old);

	if (ret < 0) {
		fprintf(stderr, "Can't clone %s\n", oldvol);
		return 1;
	}

	return 0;
}

static int do_mosaic_drop_tess(mosaic_t m, int argc, char **argv)
{
	tessera_t t;
	const char *volume;

	if (argc < 1) {
		printf("Usage: moctl <mosaic> drop <tessera>\n");
		return 1;
	}

	volume = argv[0];
	t = mosaic_open_tess(m, volume, 0);
	if (!t) {
		fprintf(stderr, "No tessera %s\n", volume);
		return 1;
	}

	if (mosaic_drop_tess(t, 0) < 0) {
		fprintf(stderr, "Can't drop %s\n", volume);
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
		fprintf(stderr, "Unknown mosaic type %s\n", name);
		ret = -1;
	}

	return ret ? 1 : 0;
}

static int do_mosaic(char *name, int argc, char **argv)
{
	mosaic_t mos;
	const char *action = argv[0];

	/* Note the order of arguments checking is important here,
	 * as abbreviations are allowed by argis().
	 */

	if (argis(action, "create"))
		return do_mosaic_create(name, argc - 1, argv + 1);

	mos = mosaic_open(name, 0);
	if (!mos) {
		fprintf(stderr, "Error opening mosaic %s\n", name);
		return 1;
	}

	if (argis(action, "mount"))
		return do_mosaic_mount(mos, argc - 1, argv + 1);
	if (argis(action, "umount"))
		return do_mosaic_umount(mos, argc - 1, argv + 1);
	if (argis(action, "new"))
		return do_mosaic_new_tess(mos, argc - 1, argv + 1);
	if (argis(action, "clone"))
		return do_mosaic_clone_tess(mos, argc - 1, argv + 1);
	if (argis(action, "drop"))
		return do_mosaic_drop_tess(mos, argc - 1, argv + 1);

	fprintf(stderr, "Unknown action: %s\n", action);
	return usage(1);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Not enough arguments!\n");
		return usage(1);
	}

	return do_mosaic(argv[1], argc - 2, argv + 2);
}
