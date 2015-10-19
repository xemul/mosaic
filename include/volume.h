#ifndef __MOSAIC_VOLUME_H__
#define __MOSAIC_VOLUME_H__
#include <regex.h>
#include <stdbool.h>

struct mosaic;

struct migrate {
	bool sending;
	int fd;
};

struct volume {
	struct mosaic *m;
	char *t_name;
	struct migrate *mig;
};

struct vol_map {
	regex_t regex;
	char *repl;
};

int parse_vol_map(struct mosaic *m, const char *key, char *val);
void free_vol_map(struct mosaic *m);
char *map_vol_name(struct mosaic *m, const char *name);
#endif
