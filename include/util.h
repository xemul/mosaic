#ifndef __MOSAIC_UTIL_H__
#define __MOSAIC_UTIL_H__
int parse_mount_opts(char *opt, int *m_flags);
unsigned long parse_blk_size(char *);
#define SECTOR_SHIFT	(9)
#endif
