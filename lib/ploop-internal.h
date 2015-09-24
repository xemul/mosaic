#ifndef _PLOOP_INTERNAL_H_
#define _PLOOP_INTERNAL_H_

#define IMG_NAME	"root.hdd"

/* A few constants stolen from ploop's include/libploop.h */
#define DDXML		"DiskDescriptor.xml"
#define TOPDELTA_UUID	"{5fbaabe3-6958-40ff-92a7-860e329aab41}"
#define UUID_SIZE	39 /* same as sizeof(TOPDELTA_UUID) */

/* ploop_uuid.c */
int ploop_uuid_generate(char *uuid, int len);

#endif // _PLOOP_INTERNAL_H_
