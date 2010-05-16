#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "dismemberd.h"

int path_is_writeable_dir (char *path) {
	int res = 0;
	int fnlen;
	char *fn;
	FILE *fd;

	fnlen = (strlen(path) + strlen(".test") + 2);
	fn = (char *)malloc(fnlen);
	snprintf(fn, fnlen, "%s/%s", path, ".test");

	fd = fopen(fn, "w");
	if (fd == NULL)
		res = 1;
	else if (fclose(fd) != 0)
		res = 1;
	else if (unlink(fn) != 0)
		res = 1;

	return res;
}

int path_is_executable (char *path) {
	int res = 0;
	struct stat buf;

	if (stat(path, &buf) != 0)
		res = 1;
	else if (! ((buf.st_mode
			& (S_IRWXU|S_IRWXG|S_IRWXO)
			& (S_IXUSR|S_IXGRP|S_IXOTH))))
		res = 1;

	return res;
}

