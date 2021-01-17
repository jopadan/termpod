#include "openat2.h"

#ifdef __USE_ATFILE

int raw_openat2(int dfd, const char *path, void *how, size_t size);
{
	int dst = syscall(__NR_openat2, dfd, path, how, size);
	return dst >= 0 ? dst : -errno;
}

int openat2(int dfd, const char* path, struct open_how *how)
{
	return raw_openat2(dfd, path, how, sizeof(*how));
}

#endif
