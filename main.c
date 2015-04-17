#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "config.h"

struct mosaic_state *ms;

static inline int argv_is(char *argv, char *is)
{
	while (1) {
		if (*argv == '\0')
			return 1;
		if (*is == '\0')
			return 0;
		if (*is != *argv)
			return 0;

		is++;
		argv++;
	}
}

static int do_list(int argc, char **argv)
{
	if (argc == 0)
		goto out;

	if (argv_is(argv[0], "mosaic"))
		return list_mosaics();
	if (argv_is(argv[0], "tessera"))
		return list_tesserae();

	printf("Unknown object %s\n", argv[0]);
out:
	return 1;
}

static int do_add(int argc, char **argv)
{
	if (argc == 0)
		goto out;

	if (argv_is(argv[0], "mosaic"))
		return add_mosaic(argc - 1, argv + 1);
	if (argv_is(argv[0], "tessera"))
		return add_tessera(argc - 1, argv + 1);

	printf("Unknown object %s\n", argv[0]);
out:
	return 1;
}

int main(int argc, char **argv)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms) {
		fprintf(stderr, "Error loading config file\n");
		return 1;
	}

	if (argc < 2)
		return 1;

	if (argv_is(argv[1], "list"))
		return do_list(argc - 2, argv + 2);
	if (argv_is(argv[1], "add"))
		return do_add(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
