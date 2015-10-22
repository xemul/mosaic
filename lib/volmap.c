#include <sys/types.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mosaic.h"
#include "volume.h"
#include "util.h"

void free_vol_map(struct mosaic *m)
{
	if (!m->vol_map)
		return;

	regfree(&m->vol_map->regex);
	free(m->vol_map->repl);
	free(m->vol_map);
	m->vol_map = NULL;
}

int parse_vol_map(struct mosaic *m, const char *key, char *val)
{
	char *regex, *repl, *extra, *saveptr;
	struct vol_map *map;
	int err, ret = -1;

	// it is guaranteed by the caller that m->vol_map == NULL

	regex = strtok_r(val, " \t", &saveptr);
	repl = strtok_r(NULL, " \t", &saveptr);
	if (!repl) {
		loge("%s: invalid %s: no replacement\n",
				__func__, key);
		goto out;
	}
	// Check there's no tail characters
	extra = strtok_r(NULL, " \t", &saveptr);
	if (extra) {
		loge("%s: invalid %s: extra characters:"
				" \"%s\"\n", __func__, key, extra);
		goto out;
	}

	m->vol_map = map = xzalloc(sizeof(struct vol_map));
	if (!map)
		goto out;

	err = regcomp(&map->regex, regex, 0);
	if (err) {
		char s[256];
		regerror(err, &map->regex, s, sizeof(s));
		loge("%s: invalid %s regex \"%s\": %s\n",
				__func__, key, regex, s);
		goto out;
	}

	map->repl = xstrdup(repl);
	if (!map->repl) {
		free_vol_map(m);
		return -1;
	}

	/* We could check here that repl is adequate to regex, i.e.
	 * its \N backrefs are all mapped to relevant regex's \(...\).
	 * Practically, it requires either an example name string
	 * (that we don't have here) or a manual parsing of regex.
	 * So, don't perform any check here, it'll be done right
	 * in map_vol_name().
	 */
	ret = 0;

out:
	if (ret)
		free_vol_map(m);
	free(val);

	return ret;
}

char *map_vol_name(struct mosaic *m, const char *name)
{
	struct vol_map *map = m->vol_map;
	const int n_matches = 10;
	regmatch_t pm[n_matches];
	const char *s = name;
	char buf[256];
	int buflen;
	int i, res;

	if (!m->vol_map)
		return xstrdup(name);

	// Initial check for bufput buffer size
	buflen = strlen(map->repl) + 1;
	if (buflen > sizeof(buf)) {
		loge("%s: buffer too short (%zd < %d)\n",
				__func__, sizeof(buf), buflen);
		return NULL;
	}
	// Initial bufput buffer fill
	memcpy(buf, map->repl, buflen);

	/* Note the below regex replace implementation only does
	 * single matching and does not allow for \N backreferences
	 * in the regex itself. It could be done but seems unnecessary,
	 * and it's simpler that way, as we only call regexec() once.
	 */
	res = regexec(&map->regex, s, n_matches, pm, 0);
	if (res == REG_NOMATCH) {
		loge("%s: can't match \"%s\" against volumeMap"
				" regex\n", __func__, name);
		/* FIXME: do we want to return the original name
		 * in case there is no match against the regex?
		 * Maybe add a special flag for such behavior?
		 */
		return NULL;
	}

	for (i = 1; i < n_matches; i++) {
		int start, finish, found;
		char mark[3] = { '\\', i + '0', '\0' };
		char *dpos;

		if (pm[i].rm_so == -1) {
			// no more matches
			break;
		}
		start = pm[i].rm_so + (s - name);
		finish = pm[i].rm_eo + (s - name);
		// find the replacement mark
		dpos = buf;
		found = 0;
		while ((dpos = strstr(dpos, mark))) {
			int l = finish - start;

			// grow used size, check it
			buflen += l;
			if (buflen > sizeof(buf)) {
				loge("%s: buffer too short"
						" (%zd < %d)\n", __func__,
						sizeof(buf), buflen);
				return NULL;
			}

			// make room for replacement ...
			memmove(dpos + l, dpos + 2,
					strlen(dpos + 2) + 1);
			// ... and do replace!
			memcpy(dpos, name + start, l);
			dpos += l;
			found++;
		}
		if (!found) {
			loge("%s: warning: \\%d not found"
					"in replacement string\n",
					__func__, i);
			// let this be a warning for now
		}
	}

	return xstrdup(buf);
}
