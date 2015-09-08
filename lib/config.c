#include <stdio.h>
#include <string.h>
#include "yaml-util.h"
#include "mosaic.h"

static int parse_layout_val(yaml_parser_t *p, char *key, void *x)
{
	struct mosaic *m = x;
	char *val;

	if (!m->m_ops || !m->m_ops->parse_layout)
		return -1;

	val = yaml_parse_scalar(p);
	if (!val)
		return -1;

	return m->m_ops->parse_layout(m, key, val);
}

static int parse_layout(yaml_parser_t *p, void *x)
{
	return yaml_parse_map_body(p, parse_layout_val, x);
}

static int parse_top_val(yaml_parser_t *p, char *key, void *x)
{
	char *val;
	struct mosaic *m = x;

	if (!strcmp(key, "type")) {
		val = yaml_parse_scalar(p);
		if (!val)
			return -1;

		m->m_ops = mosaic_find_ops(val);
		free(val);

		if (!m->m_ops)
			return -1;

		if (m->m_ops->init(m))
			return -1;

		return 0;
	}

	if (!strcmp(key, "location")) {
		val = yaml_parse_scalar(p);
		if (!val)
			return -1;

		m->m_loc = val;
		return 0;
	}

	if (!strcmp(key, "default_fs")) {
		val = yaml_parse_scalar(p);
		if (!val)
			return -1;

		m->default_fs = val;
		return 0;
	}

	if (!strcmp(key, "layout"))
		return yaml_parse_block(p, YAML_MAPPING_START_EVENT,
				YAML_MAPPING_END_EVENT, parse_layout, m);

	return -1;
}

static int parse_top(yaml_parser_t *p, void *x)
{
	return yaml_parse_map_body(p, parse_top_val, x);
}

static int parse_config(yaml_parser_t *p, void *x)
{
	return yaml_parse_block(p, YAML_MAPPING_START_EVENT, YAML_MAPPING_END_EVENT, parse_top, x);
}

int mosaic_parse_config(const char *cfg, struct mosaic *m)
{
	FILE *cfg_f;
	yaml_parser_t parser;
	int ret = -1;

	cfg_f = fopen(cfg, "r");
	if (!cfg_f)
		goto out;

	if (!yaml_parser_initialize(&parser))
		goto out_c;

	yaml_parser_set_input_file(&parser, cfg_f);
	ret = yaml_parse_all(&parser, parse_config, m);
	yaml_parser_delete(&parser);
out_c:
	fclose(cfg_f);
out:
	return ret;
}
