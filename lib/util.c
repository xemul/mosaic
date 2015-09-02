#include <stdio.h>
#include <string.h>
#include "util.h"

int scan_mounts(char *mpath, char *device)
{
	FILE *f;
	char mi[1024];

	f = fopen("/proc/self/mountstats", "r");
	if (!f)
		return -1;

	while (fgets(mi, sizeof(mi), f)) {
		char *dev, *path, *e;

		dev = mi + sizeof("device");
		e = strchr(dev, ' ');
		*e = '\0';

		path = e + 1 + sizeof("mounted on");
		e = strchr(path, ' ');
		*e = '\0';

		if (strcmp(path, mpath))
			continue;

		strcpy(device, dev);
		fclose(f);
		return 0;
	}

	fclose(f);
	return -1;
}

#ifdef DOTEST
int main(int argc, char **argv)
{
	char tdev[1024];

	if (!scan_mounts(argv[1], tdev))
		printf("Resolved [%s] into [%s]\n", argv[1], tdev);

	return 0;
}
#endif
