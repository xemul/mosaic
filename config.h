#ifndef __MOSAIC_CONFIG_H__
#define __MOSAIC_CONFIG_H__
extern struct mosaic_state *ms;
int config_update(void);

/*
 * For UAPI
 */

int mosaic_load_config(void);
#endif
