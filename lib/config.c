#include <stdio.h>
#include <string.h>
#include "util.h"
#include "yaml-util.h"
#include "mosaic.h"
#include "volume.h"

static int parse_layout_val(yaml_parser_t *p, char *key, void *x)
{
	struct mosaic *m = x;
	char *val;

	if (!m->m_ops) {
		fprintf(stderr, "%s: \"type:\" should precede \"layout:\"\n",
				__func__);
		return -1;
	}
	if (!m->m_ops->parse_layout) {
		fprintf(stderr, "%s: \"layout\" not supported by %s\n",
				__func__, m->m_ops->name);
		return -1;
	}

	val = yaml_parse_scalar(p);
	CHKVAL(key, val);

	return m->m_ops->parse_layout(m, key, val);
}

static int parse_layout(yaml_parser_t *p, void *x)
{
	return yaml_parse_map_body(p, parse_layout_val, x);
}

static int parse_top_val(yaml_parser_t *p, char *key, void *x)
{
	int ret;
	char *val;
	struct mosaic *m = x;

	if (!strcmp(key, "type")) {
		val = yaml_parse_scalar(p);
		CHKVAL(key, val);
		CHKDUP(key, m->m_ops);

		m->m_ops = mosaic_find_ops(val);

		if (!m->m_ops) {
			fprintf(stderr, "%s: unknown %s: %s\n",
					__func__, key, val);
			free(val);
			return -1;
		}
		free(val);

		if (m->m_ops->init)
			ret = m->m_ops->init(m);
		else
			ret = init_mosaic_subdir(m);
		if (ret) {
			fprintf(stderr, "%s: mosaic init failed\n",
					__func__);
			return -1;
		}

		return 0;
	}

	if (!strcmp(key, "location")) {
		val = yaml_parse_scalar(p);
		CHKVAL(key, val);
		CHKDUP(key, m->m_loc);

		m->m_loc = val;
		return 0;
	}

	if (!strcmp(key, "volumeMap")) {
		val = yaml_parse_scalar(p);
		CHKVAL(key, val);
		CHKDUP(key, m->vol_map);

		return parse_vol_map(m, key, val);
	}

	if (!strcmp(key, "default_fs")) {
		val = yaml_parse_scalar(p);
		CHKVAL(key, val);
		CHKDUP(key, m->default_fs);

		m->default_fs = val;
		return 0;
	}

	if (!strcmp(key, "layout"))
		return yaml_parse_block(p, YAML_MAPPING_START_EVENT,
				YAML_MAPPING_END_EVENT, parse_layout, m);

	fprintf(stderr, "%s: unknown top element: %s\n", __func__, key);
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
	if (!cfg_f) {
		fprintf(stderr, "%s: can't open %s: %m\n", __func__, cfg);
		goto out;
	}

	if (!yaml_parser_initialize(&parser)) {
		fprintf(stderr, "%s: can't initialize yaml parser\n",
				__func__);
		goto out_c;
	}

	yaml_parser_set_input_file(&parser, cfg_f);
	ret = yaml_parse_all(&parser, parse_config, m);
	yaml_parser_delete(&parser);
out_c:
	fclose(cfg_f);
out:
	if (ret) {
		fprintf(stderr, "%s: Can't parse %s (ret=%d)\n",
				__func__, cfg, ret);
		return -1;
	}
	if (!m->m_ops) {
		fprintf(stderr, "%s: Missing or unknown \"type\" in %s\n",
				__func__, cfg);
		return -1;
	}
	return ret;
}
