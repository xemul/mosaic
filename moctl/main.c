#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/mount.h>
#include <limits.h>
#include "uapi/mosaic.h"
#include "moctl.h"
#include "mosaic.h"

char *self_name;

int usage(int ret)
{
	printf("\n"
"Usage: moctl NAME ACTION [ARGUMENT ...]\n"
"	NAME     := mosaic name (path to .mos file)\n"
"	ACTION   := have|mount|umount|attach|detach|create|clone|drop|info\n"
"	ARGUMENT := zero or more arguments, depending on ACTION\n");

	return ret;
}

/* This allows for abbreviated commands to be used,
 * such as 'm' instead of 'mount'.
 */
static inline int argis(const char *arg, const char *is)
{
	while (1) {
		if (*arg == '\0')
			return 1;
		if (*is == '\0')
			return 0;
		if (*arg != *is)
			return 0;

		arg++;
		is++;
	}
}

static int parse_mount_flags(char *str, int *flags)
{
	char *s;

	for_each_strtok(s, str, ",") {
		if (strcmp(s, "ro") == 0) {
			*flags |= MS_RDONLY;
		} else if (strcmp(s, "rw") == 0) {
			*flags &= ~MS_RDONLY;
		} else {
			fprintf(stderr, "%s: invalid mount flag: %s\n",
					__func__, s);
			return -1;
		}
	}

	return 0;
}

static int mount_usage(int ret) {
	printf("\n"
"Usage: moctl NAME mount {VOLUME|-} MOUNTPOINT [-o FLAGS]\n"
"	NAME       := mosaic name (path to .mos file)\n"
"	VOLUME     := volume to mount; \"-\" for the mosaic itself\n"
"	MOUNTPOINT := directory to mount to\n"
"	FLAGS      := comma-separated mount flags (such as \"ro\")\n");

	return ret;
}

static int do_mosaic_mount(mosaic_t m, int argc, char **argv)
{
	const char *volume, *path;
	volume_t t = NULL;
	int i, flags = 0;

	while ((i = getopt(argc, argv, "o:")) != EOF) {
		switch (i) {
		case 'o':
			if (parse_mount_flags(optarg, &flags) < 0)
				return mount_usage(1);
			break;
		default:
			// error is printed by getopt()
			return mount_usage(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 2) {
		fprintf(stderr, "%s: too %s arguments\n", __func__,
				argc < 2 ? "few" : "many");
		return mount_usage(1);
	}

	volume = argv[0];
	if (strcmp(volume, "-")) {
		t = mosaic_open_vol(m, volume, 0);
		if (!t) {
			// error is printed by mosaic_open_vol()
			mosaic_close_vol(t);
			return 1;
		}
	}

	path = argv[1];

	if (t) {
		int ret = 0;
		if (mosaic_mount_vol(t, path, flags)) {
			perror("Can't mount volume");
			ret = 1;
		}

		mosaic_close_vol(t);

		return ret;
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
	volume_t t = NULL;

	if (argc < 3) {
		printf("Usage: moctl <name> umount <volume>|- <path>\n");
		return 1;
	}

	volume = argv[1];
	if (strcmp(volume, "-")) {
		t = mosaic_open_vol(m, volume, 0);
		if (!t)
			return 1;
	}

	path = argv[2];
	if (t) {
		int ret = 0;
		if (mosaic_umount_vol(t, path, 0)) {
			perror("Can't umount volume");
			ret = 1;
		}
		mosaic_close_vol(t);
		return ret;
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

static int do_mosaic_create_vol(mosaic_t m, int argc, char **argv)
{
	int ret;
	const char *type, *volume;
	unsigned long size;

	if (argc < 4) {
		printf("Usage: moctl <name> create fs|disk <name> <size>\n");
		return 1;
	}

	type = argv[1];
	volume = argv[2];
	size = parse_size(argv[3]);

	if (argis(type, "disk"))
		ret = mosaic_make_vol(m, volume, size, 0);
	else if (argis(type, "fs"))
		ret = mosaic_make_vol_fs(m, volume, size, 0);
	else
		return 1;

	return ret ? 1 : 0;
}

static int do_mosaic_clone_vol(mosaic_t m, int argc, char **argv)
{
	int ret;
	const char *oldvol, *newvol;
	volume_t old;

	if (argc < 3) {
		printf("Usage: moctl <name> clone <old> <new>\n");
		return 1;
	}

	oldvol = argv[1];
	newvol = argv[2];

	old = mosaic_open_vol(m, oldvol, 0);
	if (!old)
		return 1;

	ret = mosaic_clone_vol(old, newvol, 0);
	mosaic_close_vol(old);

	if (ret < 0) {
		fprintf(stderr, "Can't clone %s\n", oldvol);
		return 1;
	}

	return 0;
}

static int do_mosaic_drop_vol(mosaic_t m, int argc, char **argv)
{
	int ret;
	volume_t t;
	const char *volume;

	if (argc < 2) {
		printf("Usage: moctl <mosaic> drop <volume>\n");
		return 1;
	}

	volume = argv[1];
	t = mosaic_open_vol(m, volume, 0);
	if (!t) {
		fprintf(stderr, "No volume %s\n", volume);
		return 1;
	}

	ret = 0;
	if (mosaic_drop_vol(t, 0) < 0) {
		fprintf(stderr, "Can't drop %s\n", volume);
		ret = 1;
	}

	mosaic_close_vol(t);

	return ret;
}

static int do_mosaic_attach_vol(mosaic_t m, int argc, char **argv)
{
	const char *volume;
	volume_t t;
	char dev[NAME_MAX];
	int len, ret;

	if (argc < 2) {
		printf("Usage: moctl NAME attach VOLUME\n");
		return 1;
	}

	volume = argv[1];
	t = mosaic_open_vol(m, volume, 0);
	if (!t) {
		fprintf(stderr, "No volume %s\n", volume);
		return 1;
	}

	ret = 0;
	len = mosaic_get_vol_bdev(t, dev, sizeof(dev), 0);
	if (len < 0) {
		fprintf(stderr, "Can't attach %s\n", volume);
		ret = 1;
	}
	mosaic_close_vol(t);

	if (ret == 0)
		printf("Device: %s\n", dev);

	return ret;
}

static int do_mosaic_detach_vol(mosaic_t m, int argc, char **argv)
{
	const char *volume;
	volume_t t;
	int ret;

	if (argc < 2) {
		printf("Usage: moctl NAME detach VOLUME\n");
		return 1;
	}

	volume = argv[1];
	t = mosaic_open_vol(m, volume, 0);
	if (!t) {
		fprintf(stderr, "No volume %s\n", volume);
		return 1;
	}

	ret = 0;
	if (mosaic_put_vol_bdev(t) < 0) {
		fprintf(stderr, "Can't detach %s\n", volume);
		ret = 1;
	}

	mosaic_close_vol(t);
	return ret;
}

static int do_mosaic_info(mosaic_t mos, int argc, char **argv)
{
	unsigned long long features;

	if (argc > 1) {
		printf("Usage: moctl NAME info\n");
		return -1;
	}

	/* FIXME: add flags for what info to show */
	mosaic_get_features(mos, &features);
	printf("features:");
	if (features & MOSAIC_FEATURE_CLONE)
		printf(" clone");
	if (features & MOSAIC_FEATURE_BDEV)
		printf(" bdev");
	if (features & MOSAIC_FEATURE_DISK_SIZE_MGMT)
		printf(" volsize");
	if (features & MOSAIC_FEATURE_MIGRATE)
		printf(" migrate");
	printf("\n");
	return 0;
}

static int do_mosaic_have_vol(mosaic_t m, int argc, char **argv)
{
	const char *volume;
	int r;

	if (argc < 2) {
		printf("Usage: moctl NAME have VOLUME\n");
		return 1;
	}

	volume = argv[1];
	r = mosaic_have_vol(m, volume, 0);
	if (r < 0) {
		// error is printed by mosaic_have_vol
		return 1;
	}
	printf("%s exists: %s\n", volume, r == 1 ? "yes" : "no");

	return 0;
}

static int do_mosaic(char *name, int argc, char **argv)
{
	mosaic_t mos;
	const char *action = argv[0];

	argv[0] = self_name; // needed for getopt_long error reporting

	/* Note the order of arguments checking is important here,
	 * as abbreviations are allowed by argis().
	 */

	mos = mosaic_open(name, 0);
	if (!mos) {
		fprintf(stderr, "Error opening mosaic %s\n", name);
		return 1;
	}

	if (argis(action, "mount"))
		return do_mosaic_mount(mos, argc, argv);
	if (argis(action, "umount"))
		return do_mosaic_umount(mos, argc, argv);
	if (argis(action, "attach"))
		return do_mosaic_attach_vol(mos, argc, argv);
	if (argis(action, "detach"))
		return do_mosaic_detach_vol(mos, argc, argv);
	if (argis(action, "create"))
		return do_mosaic_create_vol(mos, argc, argv);
	if (argis(action, "clone"))
		return do_mosaic_clone_vol(mos, argc, argv);
	if (argis(action, "drop"))
		return do_mosaic_drop_vol(mos, argc, argv);
	if (argis(action, "info"))
		return do_mosaic_info(mos, argc, argv);
	if (argis(action, "have"))
		return do_mosaic_have_vol(mos, argc, argv);

	fprintf(stderr, "Unknown action: %s\n", action);
	return usage(1);
}

static void do_printf(int level, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	if (level <= LOG_WRN)
		vfprintf(stderr, f, args);
	else
		vprintf(f, args);
	va_end(args);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Not enough arguments!\n");
		return usage(1);
	}

	/*
	 * FIXME: Add --debug or -vN option
	 */
	mosaic_set_log_fn(do_printf);

	self_name = argv[0];
	return do_mosaic(argv[1], argc - 2, argv + 2);
}
