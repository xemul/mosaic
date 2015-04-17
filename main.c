#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

struct mosaic_state *ms;

int main(int argc, char **argv)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms) {
		fprintf(stderr, "Error loading config file\n");
		return 1;
	}

	if (argc < 2) {
		printf("Usage: moctl [mosaic|tessera] ...\n");
		return 1;
	}

	if (argv_is(argv[1], "mosaic"))
		return do_mosaic(argc - 2, argv + 2);
	if (argv_is(argv[1], "tessera"))
		return do_tessera(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
