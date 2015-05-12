#include <stdio.h>
#include <string.h>
#include <yaml.h>
#include "thin_id.h"
#include "yaml-util.h"

/* FIXME -- combine with STATUS_DIR from status.c */
#define THIN_MAP_DIR	"mosaic.thin.map"

void dev_map_file_name(char *dev, char *file)
{
	char *aux;

	sprintf(file, THIN_MAP_DIR "/thin_%s", dev);

	/*
	 * Mangle /-s with _-s to get valid file name
	 */
	for (aux = file + sizeof(THIN_MAP_DIR); *aux != '\0'; aux++)
		if (*aux == '/')
			*aux = '_';
}

struct thin_map {
	char *tess;
	int age;
	int vol_id;
};

struct thin_map_search {
	struct thin_map m;
	int max_id;
};

static int parse_map_elem(yaml_parser_t *p, char *key, void *x)
{
	struct thin_map *tm = x;
	char *val;

	val = yaml_parse_scalar(p);
	if (!val)
		return -1;

	if (!strcmp(key, "tessera")) {
		tm->tess = val;
		return 0;
	}

	if (!strcmp(key, "age")) {
		tm->age = atoi(val);
		free(val);
		return 0;
	}

	if (!strcmp(key, "vol_id")) {
		tm->vol_id = atoi(val);
		free(val);
		return 0;
	}

	free(val);
	return -1;
}

static int parse_map(yaml_parser_t *p, void *x)
{
	struct thin_map_search *tms = x;
	struct thin_map tm;
	int ret;

	ret = yaml_parse_map_body(p, parse_map_elem, &tm);
	if (ret < 0)
		return ret;

	if (!strcmp(tms->m.tess, tm.tess) && (tms->m.age == tm.age)) {
		if (tms->m.vol_id != -1)
			printf("BUG: double map mapping for %s:%d\n",
					tms->m.tess, tms->m.age);

		tms->m.vol_id = tm.vol_id;
	}

	if (tms->max_id < tm.vol_id)
		tms->max_id = tm.vol_id;

	free(tm.tess);

	return ret;
}

static int parse_maps(yaml_parser_t *p, void *x)
{
	return yaml_parse_block_seq(p, YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT,
			parse_map, x);
}

static int parse_maps_file(yaml_parser_t *p, void *x)
{
	return yaml_parse_block(p, YAML_SEQUENCE_START_EVENT, YAML_SEQUENCE_END_EVENT,
			parse_maps, x);
}

int thin_get_id(char *dev_name, char *tess_name, int age)
{
	char m_path[1024];
	FILE *mapf;
	yaml_parser_t p;
	int ret = -1;
	struct thin_map_search tms = {
		.m = {
			.tess = tess_name,
			.age = age,
			.vol_id = -1,
		},
		.max_id = -1,
	};

	if (!yaml_parser_initialize(&p))
		goto out;

	dev_map_file_name(dev_name, m_path);
	mapf = fopen(m_path, "a+");
	if (!mapf)
		goto out_p;

	yaml_parser_set_input_file(&p, mapf);
	ret = yaml_parse_all(&p, parse_maps_file, &tms);
	if (ret)
		goto out_f;

	if (tms.m.vol_id > 0) {
		ret = tms.m.vol_id;
		goto out_f;
	}

	ret = tms.max_id + 1;
	fprintf(mapf, "- tessera: %s\n", tess_name);
	fprintf(mapf, "  age: %d\n", age);
	fprintf(mapf, "  vol_id: %d\n", ret);

out_f:
	fclose(mapf);
out_p:
	yaml_parser_delete(&p);
out:
	return ret;
}
