#ifndef __MOSAIC_LOG_H__
#define __MOSAIC_LOG_H__
#include <errno.h>
#include "uapi/mosaic.h"

extern mosaic_log_fn print_log;

#define log(fmt, ...)							\
		do { 							\
			if (print_log)					\
				print_log("%s/%d:" fmt, 		\
					__FILE__, __LINE__,		\
					##__VA_ARGS__);			\
		} while (0)

#define loge(fmt, ...)							\
		do { 							\
			if (print_log)					\
				print_log("%s/%d error %s: " fmt, 	\
					__FILE__, __LINE__,		\
					strerror(errno),		\
					##__VA_ARGS__);			\
		} while (0)

#endif
