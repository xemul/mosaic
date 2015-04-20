#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mosaic.h"
#include "tessera.h"
#include "overlay.h"

struct overlay_tessera {
	char *ovl_location;
};

static int add_overlay(struct tess_desc *me, char *name, int argc, char **argv)
{
	struct tessera *t;
	struct overlay_tessera *ot;

	if (argc < 1) {
		printf("Usage: moctl tessera add overlay [name] [location]\n");
		return 1;
	}

	ot = malloc(sizeof(*ot));
	ot->ovl_location = strdup(argv[0]);

	t = malloc(sizeof(*t));
	t->t_name = name;
	t->t_desc = me;
	t->priv = ot;

	list_add_tail(&t->sl, &ms->tesserae);

	return 0;
}

static void del_overlay(struct tess_desc *td, struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	free(ot->ovl_location);
	free(ot);
}

static int parse_overlay(struct tessera *t, char *key, char *val)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot) {
		ot = malloc(sizeof(*ot));
		t->priv = ot;
	}

	if (!strcmp(key, "location")) {
		ot->ovl_location = strdup(val);
		return 0;
	}

	return -1;
}

static void save_overlay(struct tessera *t, FILE *f)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	fprintf(f, "    location: %s\n", ot->ovl_location);
}

static void show_overlay(struct tessera *t)
{
	struct overlay_tessera *ot = t->priv;

	if (!ot)
		return;

	printf("location: %s\n", ot->ovl_location);
}

/*
 * Overlay location layout:
 *
 * location/
 *   base/       -> base delta
 *   age-N/
 *     current/  -> for upperdir
 *     work/     -> for workdir
 *     parent   -> symlink on parent age
 *     root/     -> for mountpoint
 */

static int mount_overlay(struct tessera *t, int age, char *path, char *options)
{
	printf("NOT IMPLEMENTED\n");
	return -1;
}

struct tess_desc tess_desc_overlay = {
	.td_name = "overlay",
	.add = add_overlay,
	.del = del_overlay,
	.parse = parse_overlay,
	.save = save_overlay,
	.show = show_overlay,
	.mount = mount_overlay,
};
