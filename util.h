#ifndef __MOSAIC_UTIL_H__
#define __MOSAIC_UTIL_H__
static inline int argv_is(char *argv, char *is)
{
	while (1) {
		if (*argv == '\0')
			return 1;
		if (*is == '\0')
			return 0;
		if (*is != *argv)
			return 0;

		is++;
		argv++;
	}
}
#endif
