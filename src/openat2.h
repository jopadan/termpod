#ifndef _OPENAT2_H
#define _OPENAT2_H

#ifdef __USE_ATFILE

int raw_openat2(int dfd, const char *path, void *how, size_t size);
int sys_openat2(int dfd, const char *path, struct open_how *how);
int openat2(int dfd, const char* path, struct open_how *how);
#endif

#endif
