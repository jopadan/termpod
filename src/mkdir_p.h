#ifndef _MKDIR_P_H
#define _MKDIR_P_H

#ifndef __USE_ATFILE
#define __USE_ATFILE 1
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <ctype.h>

/* recursive mkdir */
int mkdir_p(const char *pathname, const mode_t mode);

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

/* recursive mkdirat based on openat2 */
int mkdirat_p(int fd, const char *pathname, const mode_t mode);

#endif /* __USE_ATFILE */

#endif /* _MKDIR_P_H */
