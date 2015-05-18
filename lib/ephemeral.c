#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include "mosaic.h"
#include "tessera.h"
#include "log.h"
#include "config.h"

struct eph_tessera {
	int n_dirs;
	char **dirs;
};

static void parse_dirs(struct eph_tessera *et, char *str)
{
	char *aux;

	while (1) {
		int i;

		aux = strchr(str, ':');
		if (aux)
			*aux = '\0';

		i = et->n_dirs++;
		et->dirs = realloc(et->dirs, et->n_dirs * sizeof(char *));
		et->dirs[i] = strdup(str);

		if (!aux)
			break;

		str = aux + 1;
	}
}

static int add_eph(struct tessera *t, int n_opts, char **opts)
{
	struct eph_tessera *et;

	if (n_opts < 1) {
		log("Usage: moctl tessera add <name> ephemeral <dirs>:\n");
		return -1;
	}

	t->priv = et = malloc(sizeof(*et));
	et->n_dirs = 0;
	et->dirs = NULL;

	parse_dirs(et, opts[0]);

	return 0;
}

static void del_eph(struct tessera *t, int age)
{
	int i;
	struct eph_tessera *et = t->priv;

	for (i = 0; i < et->n_dirs; i++)
		free(et->dirs[i]);

	free(et->dirs);
	free(et);
}

static int parse_eph(struct tessera *t, char *key, char *val)
{
	struct eph_tessera *et = t->priv;

	if (!et) {
		et = malloc(sizeof(*et));
		et->n_dirs = 0;
		et->dirs = NULL;
		t->priv = et;
	}

	if (!key)
		return et->n_dirs == 0 ? -1 : 0;

	if (!strcmp(key, "dirs")) {
		parse_dirs(et, val);
		return 0;
	}

	return -1;
}

static inline void print_eph_info(FILE *f, int off, struct eph_tessera *et)
{
	int i;

	fprintf(f, "%*sdirs:", off, "");
	for (i = 0; i < et->n_dirs; i++)
		fprintf(f, "%c%s", (i == 0 ? ' ' : ':'), et->dirs[i]);
	fprintf(f, "\n");
}

static void save_eph(struct tessera *t, FILE *f)
{
	print_eph_info(f, CFG_TESS_OFF, t->priv);
}

static void show_eph(struct tessera *t, int age)
{
	if (age == -1)
		print_eph_info(stdout, 0, t->priv);
}

static int mount_eph(struct tessera *t, int age, char *path, char *options)
{
	struct eph_tessera *et = t->priv;
	char aux[PATH_MAX];
	int l, i;

	if (age != 0)
		return -1;

	if (options) {
		log("Mount options are not yet supported\n");
		return -1;
	}

	l = sprintf(aux, "%s/", path);
	if (mount("none", aux, "tmpfs", 0, NULL))
		return -1;

	for (i = 0; i < et->n_dirs; i++) {
		sprintf(aux + l, "%s", et->dirs[i]);
		if (mkdir(aux, 0600))
			goto err;
	}

	return 0;

err:
	aux[l] = '\0';
	umount(aux);
	return -1;
}

struct tess_desc tess_desc_ephemeral = {
	.td_name = "ephemeral",
	.add = add_eph,
	.del = del_eph,
	.parse = parse_eph,
	.save = save_eph,
	.show = show_eph,
	.mount = mount_eph,
	/*
	 * Growing can not be implemented at all
	 */
};
