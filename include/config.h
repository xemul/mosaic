#ifndef __MOSAIC_CONFIG_H__
#define __MOSAIC_CONFIG_H__
extern struct mosaic_state *ms;
int config_update(void);

/*
 * Offset in yaml config file with which tessera-specific
 * info is printed.
 */
#define CFG_TESS_OFF	4
#endif
