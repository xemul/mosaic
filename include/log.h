#ifndef __MOSAIC_LOG_H__
#define __MOSAIC_LOG_H__
#include <errno.h>
#include "uapi/mosaic.h"

#ifndef DOTEST
extern mosaic_log_fn log_fn;
extern enum log_level log_level;

#define print_log(level, fmt, ...)					\
		do { 							\
			if (log_fn && level <= log_level)		\
				log_fn(level, "%s/%d:" fmt, 		\
					__FILE__, __LINE__,		\
					##__VA_ARGS__);			\
		} while (0)
#else
#define print_log(level, fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#define logd(fmt, ...)		print_log(LOG_DBG, "debug: " fmt, ##__VA_ARGS__)
#define log(fmt, ...)		print_log(LOG_INF, fmt, ##__VA_ARGS__)
#define logw(fmt, ...)		print_log(LOG_WRN, "WARN: " fmt, ##__VA_ARGS__)
#define loge(fmt, ...)		print_log(LOG_ERR, "ERROR: " fmt, ##__VA_ARGS__)

#endif
