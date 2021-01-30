#ifndef _OPENAT2_H
#define _OPENAT2_H

#ifdef __USE_ATFILE
#include <linux/openat2.h>

#ifndef SYS_openat2

#ifndef __NR_openat2
#define __NR_openat2 437
#endif /* __NR_openat2 */

#define SYS_openat2 __NR_openat2

#endif /* SYS_openat2 */

/* openat2 wrapper */
int raw_openat2(int dfd, const char *path, void *how, size_t size);
int sys_openat2(int dfd, const char *path, struct open_how *how);
int openat2(int dfd, const char* path, struct open_how *how);
#endif

#endif
