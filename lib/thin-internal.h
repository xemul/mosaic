#ifndef __MOSAIC_THIN_H__
#define __MOSAIC_THIN_H__
/*
 * All routines return 1 if v_name found in DB, 0 if
 * not, <0 on error.
 *
 * The _new one if returned 0 generates and report new
 * ID into the *ret_id variable.
 */
int thin_id_new(const char *m_name, const char *v_name, unsigned *ret_id);
int thin_id_get(const char *m_name, const char *v_name, unsigned *ret_id);
int thin_id_del(const char *m_name, const char *v_name);
#endif
