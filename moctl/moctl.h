#ifndef __MOSAIC_MOCTL_H__
#define __MOSAIC_MOCTL_H__
struct mosaic;

#define for_each_strtok(p, str, sep)				\
	for (p = strtok(str, sep); p; p = strtok(NULL, sep))

#endif
