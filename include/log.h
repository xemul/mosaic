#ifndef __MOSAIC_LOG_H__
#define __MOSAIC_LOG_H__
#define log(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#define loge(fmt, ...)	fprintf(stderr, fmt, ##__VA_ARGS__)
#endif
