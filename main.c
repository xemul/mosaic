#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

struct mosaic_state *ms;

static void usage(void)
{
	printf(
"Usage: moctl <object> <action> [<options>]\n"
"\n"
" * Objects:\n"
"    m|mosaic\n"
"    t|tessera\n"
"\n"
" * Actions:\n"
"   Common:\n"
"    l|list\n"
"    s|show    <name>\n"
"\n"
"   Mosaic:\n"
"    a|add     <name> [<elements>]\n"
"    d|del     <name>\n"
"    c|change  <name> [<elements>]\n"
"    m|mount   <name> <location> [<mount-options>]\n"
"\n"
"     Element: <name>:<age>:<location>[:options=<mount-options>]\n"
"              for \"change\" <name>:del to remove\n"
"\n"
"   Tessera:\n"
"    a|add     <name> <type> [<type-options>]\n"
"    d|del     <name>\n"
"    m|mount   <name>:<age> <location> [<mount-options>]\n"
"    g|grow    <name> <new-age>[:<base-age>]\n"
);
}

int main(int argc, char **argv)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms) {
		fprintf(stderr, "Error loading config file\n");
		return 1;
	}

	if (argc < 2) {
		usage();
		return 1;
	}

	if (argv_is(argv[1], "mosaic"))
		return do_mosaic(argc - 2, argv + 2);
	if (argv_is(argv[1], "tessera"))
		return do_tessera(argc - 2, argv + 2);

	printf("Unknown action %s\n", argv[1]);
	return 1;
}
