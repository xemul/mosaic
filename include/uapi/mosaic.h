#ifndef __MOSAIC_UAPI_H__
#define __MOSAIC_UAPI_H__

/* Misc */
typedef void (*mosaic_log_fn)(const char *f, ...)
	            __attribute__ ((format(printf, 1, 2)));
#endif
