#include <stdio.h>
#include "mosaic.h"

static char *t_types[] = {
	[ TESSERA_OVERLAY ] = "overlay",
	[ TESSERA_BTRFS ] = "btrfs",
	[ TESSERA_DMTHIN ] = "dmthin",
};

static void show_tessera(struct tessera *t)
{
	printf("name: %s\n", t->t_name);
	printf("type: %s\n", t_types[t->t_type]);
	printf("\n");
}

int list_tesserae(void)
{
	struct tessera *t;

	list_for_each_entry(t, &ms->tesserae, sl)
		show_tessera(t);

	return 0;
}

int add_tessera(int argc, char **argv)
{
	return 1;
}
