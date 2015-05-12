#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <yaml.h>
#include "yaml-util.h"
#include "mosaic.h"
#include "tessera.h"

struct mosaic_state *ms;

#define err(args...)	fprintf(stderr, ##args)

/******************************************************
 * Mosaic config parser
 ******************************************************/

static inline struct mosaic *last_mosaic(struct mosaic_state *ms)
{
	return list_entry(ms->mosaics.prev, struct mosaic, sl);
}

static inline struct element *last_element(struct mosaic_state *ms)
{
	struct mosaic *m;

	m = last_mosaic(ms);
	return list_entry(m->elements.prev, struct element, ml);
}

static inline struct tessera *last_tesera(struct mosaic_state *ms)
{
	return list_entry(ms->tesserae.prev, struct tessera, sl);
}

static int parse_tessera_value(yaml_parser_t *p, char *key, void *x)
{
	struct tessera *t;
	char *val;

	val = yaml_parse_scalar(p);
	if (!val)
		return -1;

	t = last_tesera(x);
	if (!strcmp(key, "name")) {
		t->t_name = val;
		return 0;
	}

	if (!strcmp(key, "type")) {
		t->t_desc = tess_desc_by_type(val);
		if (!t->t_desc) {
			err("Unknown tessera type %s\n", val);
			return -1;
		}

		free(val);
		return 0;
	}

	if (t->t_desc && t->t_desc->parse) {
		/*
		 * FIXME -- if type goes after other options,
		 * why do we need to fail?
		 */
		if (!t->t_desc->parse(t, key, val))
			return 0;
	}

	err("Unknown tessera field %s\n", key);
	free(val);
	return -1;
}

static int parse_tessera(yaml_parser_t *p, void *x)
{
	struct mosaic_state *ms = x;
	struct tessera *t;
	int ret;

	t = malloc(sizeof(*t));
	t->t_name = NULL;
	t->t_desc = NULL;
	list_add_tail(&t->sl, &ms->tesserae);

	ret = yaml_parse_map_body(p, parse_tessera_value, x);

	if (t->t_desc && t->t_desc->parse && t->t_desc->parse(t, NULL, NULL))
		return -1;

	return ret;
}

static int parse_tesserae(yaml_parser_t *p, void *x)
{
	return yaml_parse_block_seq(p, "tessera", YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT,
			parse_tessera, x);
}

static int parse_element_value(yaml_parser_t *p, char *key, void *x)
{
	char *val;
	struct element *e;

	val = yaml_parse_scalar(p);
	if (!val)
		return -1;

	e = last_element(x);
	if (!strcmp(key, "name")) {
		e->e_name = val;
		return 0;
	}

	if (!strcmp(key, "age")) {
		e->e_age = atoi(val);
		free(val);
		return 0;
	}

	if (!strcmp(key, "at")) {
		e->e_at = val;
		return 0;
	}

	if (!strcmp(key, "options")) {
		e->e_options = val;
		return 0;
	}

	err("Unknown element field %s\n", key);
	free(val);
	return -1;
}

static int parse_element(yaml_parser_t *p, void *x)
{
	struct mosaic *m;
	struct element *e;

	m = last_mosaic(x);

	e = malloc(sizeof(*e));
	e->t = NULL;
	e->e_age = 0;
	e->e_at = NULL;
	e->e_options = NULL;
	list_add_tail(&e->ml, &m->elements);

	return yaml_parse_map_body(p, parse_element_value, x);
}

static int parse_elements(yaml_parser_t *p, void *x)
{
	return yaml_parse_block_seq(p, "element", YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT,
			parse_element, x);
}

static int parse_mosaic_value(yaml_parser_t *p, char *key, void *x)
{
	struct mosaic *m;
	char *val;

	if (!strcmp(key, "elements"))
		return yaml_parse_block(p, YAML_SEQUENCE_START_EVENT, YAML_SEQUENCE_END_EVENT,
				parse_elements, x);

	val = yaml_parse_scalar(p);
	if (!val)
		return -1;

	m = last_mosaic(x);
	if (!strcmp(key, "name")) {
		m->m_name = val;
		return 0;
	}

	err("Unknown mosaic field %s\n", key);
	free(val);
	return -1;
}

static int parse_mosaic(yaml_parser_t *p, void *x)
{
	struct mosaic_state *ms = x;
	struct mosaic *m;

	m = malloc(sizeof(*m));
	m->m_name = NULL;
	INIT_LIST_HEAD(&m->elements);
	list_add_tail(&m->sl, &ms->mosaics);

	return yaml_parse_map_body(p, parse_mosaic_value, x);
}

static int parse_mosaics(yaml_parser_t *p, void *x)
{
	return yaml_parse_block_seq(p, "mosaic", YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT,
			parse_mosaic, x);
}

static int parse_top_value(yaml_parser_t *p, char *key, void *x)
{
	if (!strcmp(key, "tesserae"))
		return yaml_parse_block(p, YAML_SEQUENCE_START_EVENT, YAML_SEQUENCE_END_EVENT,
				parse_tesserae, x);

	if (!strcmp(key, "mosaics"))
		return yaml_parse_block(p, YAML_SEQUENCE_START_EVENT, YAML_SEQUENCE_END_EVENT,
				parse_mosaics, x);

	return -1;
}

static int parse_top(yaml_parser_t *p, void *x)
{
	return yaml_parse_map_body(p, parse_top_value, x);
}

static int parse_document(yaml_parser_t *p, void *x)
{
	return yaml_parse_block(p, YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT,
			parse_top, x);
}

static int parse_stream(yaml_parser_t *p, void *x)
{
	return yaml_parse_block(p, YAML_DOCUMENT_START_EVENT, YAML_DOCUMENT_END_EVENT,
			parse_document, x);
}

static int resolve_tesserae(struct mosaic_state *ms)
{
	struct mosaic *m;
	struct element *e;
	struct tessera *t;

	list_for_each_entry(m, &ms->mosaics, sl) {
		list_for_each_entry(e, &m->elements, ml) {
			t = find_tessera(ms, e->e_name);
			if (!t) {
				err("Can't find tessera %s for mosaic %s\n",
						e->e_name, m->m_name);
				return -1;
			}

			e->t = t;
		}
	}

	return 0;
}

static struct mosaic_state *mosaic_parse_config(char *cfg_file)
{
	FILE *f;
	yaml_parser_t parser;
	int ret = -1;
	struct mosaic_state *ms = NULL;

	f = fopen(cfg_file, "r");
	if (!f)
		goto out;

	if (!yaml_parser_initialize(&parser))
		goto out_c;

	ms = malloc(sizeof(*ms));
	INIT_LIST_HEAD(&ms->tesserae);
	INIT_LIST_HEAD(&ms->mosaics);

	yaml_parser_set_input_file(&parser, f);

	ret = yaml_parse_block(&parser, YAML_STREAM_START_EVENT, YAML_STREAM_END_EVENT,
			parse_stream, ms);
	if (!ret)
		ret = resolve_tesserae(ms);

	if (ret) {
		free(ms);
		ms = NULL;
	}

	yaml_parser_delete(&parser);
out_c:
	fclose(f);
out:
	return ms;
}

int mosaic_load_config(void)
{
	ms = mosaic_parse_config("mosaic.conf");
	if (!ms)
		return -1;

	return 0;
}

#define UPD_CFG_NAME	".mosaic.conf.new"

int config_update(void)
{
	FILE *f;
	struct tessera *t;
	struct mosaic *m;

	f = fopen(UPD_CFG_NAME, "w");
	if (!f) {
		err("Can't update config");
		return 1;
	}

	if (!list_empty(&ms->tesserae))
		fprintf(f, "tesserae:\n");

	list_for_each_entry(t, &ms->tesserae, sl) {
		fprintf(f, "  - name: %s\n", t->t_name);
		fprintf(f, "    type: %s\n", t->t_desc->td_name);
		if (t->t_desc->save)
			t->t_desc->save(t, f);

		fprintf(f, "\n");
	}

	if (!list_empty(&ms->mosaics))
		fprintf(f, "mosaics:\n");

	list_for_each_entry(m, &ms->mosaics, sl) {
		struct element *e;

		fprintf(f, "  - name: %s\n", m->m_name);
		if (!list_empty(&m->elements))
			fprintf(f, "    elements:\n");

		list_for_each_entry(e, &m->elements, ml) {
			fprintf(f, "      - name: %s\n", e->t->t_name);
			if (e->e_age != 0)
				fprintf(f, "        age: %d\n", e->e_age);
			if (e->e_at)
				fprintf(f, "        at: %s\n", e->e_at);
			if (e->e_options)
				fprintf(f, "        options: %s\n", e->e_options);
			fprintf(f, "\n");
		}
	}

	fclose(f);

	if (rename(UPD_CFG_NAME, "mosaic.conf")) {
		perror("Can't rename config");
		return 1;
	}

	return 0;
}
