/* recursive mkdir based on
http://nion.modprobe.de/blog/archives/357-Recursive-directory-creation.html
*/
#include "mkdir_p.h"

/* recursive mkdir */
int mkdir_p(const char *pathname, const mode_t mode) 
{
    char tmp[PATH_MAX];
    char *p = NULL;
    struct stat sb;
    size_t len;
    
    /* copy path */
    len = strnlen (pathname, PATH_MAX);
    if (len == 0 || len == PATH_MAX) {
        return -1;
    }
    memcpy (tmp, pathname, len);
    tmp[len] = '\0';

    /* remove trailing slash */
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* check if path exists and is a directory */
    if (stat (tmp, &sb) == 0) {
        if (S_ISDIR (sb.st_mode)) {
            return 0;
        }
    }
    
    /* recursive mkdir */
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            /* test path */
            if (stat(tmp, &sb) != 0) {
                /* path does not exist - create directory */
                if (mkdir(tmp, mode) < 0) {
                    return -1;
                }
            } else if (!S_ISDIR(sb.st_mode)) {
                /* not a directory */
                return -1;
            }
            *p = '/';
        }
    }
    /* test path */
    if (stat(tmp, &sb) != 0) 
    {
        /* path does not exist - create directory */
        if (mkdir(tmp, mode) < 0)
	{
            return -1;
        }
    } 
    else if (!S_ISDIR(sb.st_mode))
    {
        /* not a directory */
        return -1;
    }
    return 0;
}

#ifdef __USE_ATFILE


/* recursive mkdirat */
int mkdirat_p(int fd, const char *pathname, const mode_t mode)
{
    /* assert correct arguments */
    if(pathname == NULL)
    {
    	errno = EINVAL;
	return -1;
    }

    if(!*pathname)
    {
    	errno = ENOENT;
	return -1;
    }

    if(strlen(pathname) >= PATH_MAX)
    {
    	errno = ENAMETOOLONG;
	return -1;
    }

    char path[PATH_MAX];
    char *p = NULL;
    if(isabspath(pathname))
    {
    	p = stpcpy(path, pathname);
    }

    struct open_how *how = calloc(1, sizeof(struct open_how)); 
    how->flags = O_CREAT;
    how->mode = mode;
    how->resolve = isabspath(pathname) ? RESOLVE_IN_ROOT : RESOLVE_BENEATH;

    return sys_openat2(fd, path, how);
}
#endif
