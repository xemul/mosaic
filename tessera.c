#include <stdio.h>
#include <stdlib.h>
#include "mosaic.h"
#include "tessera.h"
#include "config.h"
#include "util.h"

static int add_overlay(struct tess_desc *me, char *name, int argc, char **argv)
{
	struct tessera *t;

	if (argc != 0) {
		printf("Unknown args for overlay %s...\n", argv[0]);
		return 1;
	}

	t = malloc(sizeof(*t));
	t->t_name = name;
	t->t_desc = me;
	list_add_tail(&t->sl, &ms->tesserae);

	return 0;
}

static struct tess_desc tess_desc_overlay = {
	.td_name = "overlay",
	.add = add_overlay,
};

static struct tess_desc tess_desc_btrfs = {
	.td_name = "btrfs",
};

static struct tess_desc tess_desc_dmthin = {
	.td_name = "dmthin",
};

struct tess_desc *tess_desc_by_type(char *type)
{
	if (!strcmp(type, "overlay"))
		return &tess_desc_overlay;
	if (!strcmp(type, "btrfs"))
		return &tess_desc_btrfs;
	if (!strcmp(type, "dm_thin"))
		return &tess_desc_dmthin;

	return NULL;
}

static void show_tessera(struct tessera *t)
{
	printf("name: %s\n", t->t_name);
	printf("type: %s\n", t->t_desc->td_name);
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
	int i;
	struct tess_desc *td;

	if (argc < 2) {
		printf("Usage mosaic add tessera [type] [name] ...\n");
		goto out;
	}

	td = tess_desc_by_type(argv[0]);
	if (!td || !td->add)
		goto out_t;

	if (td->add(td, argv[1], argc - 2, argv + 2))
		return 1;

	return config_update();

out_t:
	printf("Unknown tessera type %s\n", argv[0]);
out:
	return 1;
}
