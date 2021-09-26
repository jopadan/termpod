#include "pod_common.h"
#include "libcrcle/crcle.h"

/* rotate chars for progress status */
char *rotorchar = "-/|\\";

const char POD_IDENT[POD_IDENT_TYPE_SIZE][POD_IDENT_SIZE + 1] = {
	"\0POD1", "POD2\0", "POD3\0",
       	"POD4\0", "POD5\0", "POD6\0", "dtxe\0",
};

const char POD_FORMAT_DESCRIPTION[POD_IDENT_TYPE_SIZE][POD_STRING_32] = {
"POD1", "POD2", "POD3", "POD4", "POD5", "POD6", "Extended POD (EPD)",
};

uint32_t pod_crc(pod_byte_t* data, pod_size_t count)
{
	if(data == NULL || count == 0)
	{
		fprintf(stderr, "ERROR: pod_crc() data == NULL or count == 0!");
		return 0;
	}
	return ccitt32_updcrc(0xffffffff, data, count);
}

const ssize_t POD_HEADER_SIZE[POD_IDENT_TYPE_SIZE] =
{
	POD_HEADER_POD1_SIZE,
	POD_HEADER_POD2_SIZE,
	POD_HEADER_POD3_SIZE,
	POD_HEADER_POD4_SIZE,
	POD_HEADER_POD5_SIZE,
	POD_HEADER_POD6_SIZE,
	POD_HEADER_EPD_SIZE,
};

const ssize_t POD_DIR_ENTRY_SIZE[POD_IDENT_TYPE_SIZE] =
{
	POD_DIR_ENTRY_POD1_SIZE,
	POD_DIR_ENTRY_POD2_SIZE,
	POD_DIR_ENTRY_POD3_SIZE,
	POD_DIR_ENTRY_POD4_SIZE,
	POD_DIR_ENTRY_POD5_SIZE,
	POD_DIR_ENTRY_POD6_SIZE,
	POD_DIR_ENTRY_EPD_SIZE,
};

int pod_size_compare(const void* a, const void* b)
{
	const pod_size_t x = *(pod_size_t*)a;
	const pod_size_t y = *(pod_size_t*)b;
	return x == y ? 0 : x < y ? -1 : 1;
}


pod_ident_type_t pod_type(char* ident)
{
	for(int i = 0; i < POD_IDENT_TYPE_SIZE; i++)
	{
		if (strncmp(ident, POD_IDENT[i], POD_IDENT_SIZE) == 0)
			return i;
	}
  	return POD1;
}

const char* pod_type_str(pod_ident_type_t type)
{
	return POD_IDENT[type];
}

bool is_pod(char* ident)
{
  return (POD_IDENT_TYPE_SIZE > pod_type(ident) >= 0);
}

pod_ident_type_t pod_type_peek(pod_path_t path)
{
	FILE* file = fopen(path, "rb");
	if(!file)
		return POD_IDENT_TYPE_SIZE;

	pod_char_t ident[POD_IDENT_SIZE + 1] = { 0 };

	if(fread(ident, 1, POD_IDENT_SIZE, file) != POD_IDENT_SIZE)
	{
		fclose(file);
		return POD_IDENT_TYPE_SIZE;
	}
	fclose(file);

	return pod_type(ident);
}

pod_string_t pod_type_to_file_ext(int pod_type)
{
	switch(pod_type)
	{
		case EPD:
			return "EPD";
		default:
			return "POD";
	}

	return NULL;
}

/* time_t convert functions */
time_t pod_time32_to_time(pod_time32_t t32)
{
	return((time_t)t32);
}

time_t pod_time64_to_time(pod_time64_t t64)
{
	return((time_t)t64);
}
pod_time32_t pod_time_to_time32(time_t t)
{
	return((pod_time32_t)t);
}

pod_time64_t pod_time_to_time64(time_t t)
{
	return((pod_time64_t)t);
}

pod_string_t pod_ctime(pod_time_t* time32)
{
	time_t time = pod_time32_to_time(*time32);

	char* str = ctime(&time);
	str[strcspn(str, "\n")] = '\0';

	return  str;
}

pod_dir_t pod_opendir_mkdir_p(const pod_string_t path, mode_t* mode)
{
	/* assert valid arguments */
	if(path == NULL)
	{
		fprintf(stderr, "ERROR: pod_opendir_mkdir_p(NULL, \'%u\'): path == NULL!\n", *mode);
		return NULL;
	}

	if(strcmp(path, "") == 0)
	{
		fprintf(stderr, "ERROR: pod_opendir_mkdir_p(\"%s\", \'%u\'): trying to create an empty directory!\n", path, *mode);
		return NULL;
	}

	/* create directory with parents */
	if(mkdir_p(path, ACCESSPERMS) != 0 && errno != EEXIST && errno != 0)
	{
		fprintf(stderr,"ERROR: mkdir_p(\"%s\", ACCESSPERMS) failed with errno: %s\n", path, strerror(errno));
		return NULL;
	}

	/* canonicalize_file_name */
	pod_path_t dir = realpath(path, NULL);
	if(dir == NULL)
	{
		fprintf(stderr, "ERROR: realpath(\"%s\", NULL) failed with errno %s\n", path, strerror(errno));
		return NULL;
	}

	/* open canoncicalized_file_name as directory */
       	pod_dir_t dst = opendir(dir);
	if(dst == NULL)
		fprintf(stderr,"ERROR: opendir(\"%s\") failed with errno %s\n", dir, strerror(errno));

	/* cleanup and return directory */
	free(dir);
	return dst;
}

pod_file_t pod_fopen_mkdir_p(pod_path_t path, const char* mode)
{
	/* assert valid arguments */
	if(path == NULL)
	{
		fprintf(stderr, "ERROR: pod_fopen_mkdir_p(NULL, %s): path == NULL!\n", mode);
		return NULL;
	}

	if(strcmp(path,"") == 0)
	{
		fprintf(stderr, "ERROR: pod_fopen_mkdir_p(\"%s\", %s): trying to open an empty path!\n", path, mode);
		return NULL;
	}

	/* split path into dirname/filename */
	pod_path_t dir = dirname(path);
	pod_path_t file = basename(path);

	/* create recursive directory with parents */
	if(!mkdir_p(dir, ACCESSPERMS))
	{
		fprintf(stderr,"ERROR: creating recursive directory %s failed with errno %s\n", dir, strerror(errno));
		free(dir);
		free(file);
		return NULL;
	}

	/* canonicalize_file_name of created path */
	pod_char_t *norm = realpath(dir, NULL);
	norm = reallocarray(norm, sizeof(dir) + sizeof(file), POD_CHAR_SIZE);

	/* add separator if necessary */
	if(norm[strlen(dir) - 1] != '/')
	{
		norm[strlen(dir)] = '/';
		norm[strlen(dir)+1] = '\0';
	}

	/* concatenate canonicalized directory name and filename */
	norm = strncat(norm, file, strlen(file));

	/* open canonicalized filepath */
	FILE* dst = fopen(norm, mode);

	/* cleanup and return file */
	free(norm);
	free(dir);
	free(file);
	return dst;
}
/*
pod_dir_t pod_opendir_mkdirat_p(int fd, pod_string_t path, const mode_t* mode)
{
	if(path == NULL)
	{
		fprintf(stderr, "ERROR: pod_opendir_mkdirat_p(%d, NULL, \'%u\'): path == NULL!\n", fd, *mode);
		return NULL;
	}

	if(strcmp(path, "") == 0)
	{
		fprintf(stderr, "ERROR: pod_opendir_mkdirat_p(%d, \"%s\", \'%u\'): trying to create an empty directory!\n", fd, path, *mode);
		return NULL;
	}

	if(mkdirat_p(fd, path, ACCESSPERMS) != 0 && errno != EEXIST && errno != 0)
	{
		fprintf(stderr,"ERROR: mkdirat_p(%d, \"%s\", ACCESSPERMS) failed with errno: %s\n", fd, path, strerror(errno));
		return NULL;
	}

	pod_path_t dir = realpath(path, NULL);
	if(dir == NULL)
	{
		fprintf(stderr, "ERROR: realpath(\"%s\", NULL) failed with errno %s\n", path, strerror(errno));
		return NULL;
	}

       	pod_dir_t dst = opendir(dir);
	if(dst == NULL)
		fprintf(stderr,"ERROR: opendir(\"%s\") failed with errno %s\n", dir, strerror(errno));

	free(dir);
	return dst;
}

pod_file_t pod_fopen_mkdirat_p(int fd, pod_path_t path, mode_t mode)
{
	if(path == NULL)
	{
		fprintf(stderr, "ERROR: pod_fopen_mkdir_p(NULL, ACCESSPERMS): path == NULL!\n", path);
		return NULL;
	}

	if(strcmp(path,"") == 0)
	{
		fprintf(stderr, "ERROR: pod_fopen_mkdir_p(\"%s\", ACCESSPERMS) trying to open an empty path!\n", path);
		return NULL;
	}

	pod_path_t dir = dirname(path);
	pod_path_t file = basename(path);

	if(mkdirat_p(fd, dir, ACCESSPERMS))
	{
		fprintf(stderr,"ERROR: creating recursive directory %s failed with errno %s\n", path, strerror(errno));
		return NULL;
	}

	pod_char_t *norm = realpath(dir, NULL);
	norm = reallocarray(norm, sizeof(dir) + sizeof(file), POD_CHAR_SIZE);
	norm = strncat(norm, file, sizeof(file));

	FILE* dst = fopen(norm, "a+");
	free(norm);
	free(dir);
	return dst;
}
*/

pod_path_t pod_path_system_home()
{
#if defined(__WIN32__) || defined(__WIN64__)
	pod_path_t home = calloc(1, POD_SYSTEM_PATH_SIZE);
	snprintf(home, POD_SYSTEM_PATH_SIZE, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
	pod_path_t home = strdup("/home");
#endif
	return home;
}

pod_path_t pod_path_system_root()
{
#if defined(__WIN32__) || defined(__WIN64__)
	pod_path_t root = calloc(1, POD_SYSTEM_PATH_SIZE);
	snprintf(root, POD_SYSTEM_PATH_SIZE, "%s%c", getenv("SYSTEMDRIVE"), POD_PATH_SEPARATOR);
#else
	pod_path_t root = strdup("/");
#endif
	return root;
}

pod_char_t pod_path_system_drive()
{
#if defined(__WIN32__) || defined(__WIN64__)
	return *getenv("SYSTEMDRIVE");
#else
	return '/';
#endif
}

pod_path_t pod_path_posix_to_win32(pod_path_t src, pod_char_t separator, pod_bool_t absolute, pod_char_t drive)
{
	if(src == NULL)
		return NULL;

	absolute = (src[0] == '/') ? true : absolute;
	pod_number_t start = absolute ? (src[0] == '/' ? 2 : 3) : 0;
	pod_number_t size = strlen(src) + start + 1;
	pod_path_t path = calloc(1, size);
	if(!strncpy(&path[start], src, strlen(src)))
	{
		fprintf(stderr, "ERROR: pod_path_posix_to_win32() strncpy() failed");
		free(path);
		return NULL;
	}

	pod_path_t sep = &path[start];

	for(int i = 0; i < size; i++)
	{
		if(path[i] == '/')
		       path[i] = separator;
	}

	path[size] = POD_PATH_NULL;

	if(absolute)
	{
		path[0] = drive;
		path[1] = ':';
		path[2] = separator;
	}

	return &path[0];
}

extern pod_path_t pod_path_trim(pod_path_t src)
{
	pod_path_t dst = strdup(src);
	pod_path_t sep = strrchr(dst, '.');
	*sep = '\0';

	dst = reallocarray(dst, dst + strlen(src) + 1 - sep, POD_CHAR_SIZE);
	return dst;
}

extern pod_path_t pod_path_to_win32(pod_path_t src, pod_bool_t absolute)

{
	if(src == NULL || pod_path_is_pod(src) == false)
		return NULL;

	pod_path_t dst = calloc(strlen(src) + 4, POD_CHAR_SIZE);

	pod_char_t root = pod_path_system_drive();
	dst[0] = root;
	dst[1] = ':';
	dst[2] = '\\';
	

	absolute = (src[0] == root) ? true : absolute;

	return dst;
}

extern pod_path_t pod_path_to_posix(pod_path_t src)
{
	if(src == NULL || pod_path_is_pod(src) == false)
		return NULL;

	pod_path_t dst = strdup(src);
	for(int i = 0; i < strlen(src); i++)
	{
		if(dst[i] == '\\')
			dst[i] = '/';
	}

	return dst;
}

bool pod_path_is_posix(pod_path_t path)
{
	for(int i = 0; i < strlen(path); i++)
	{
		if(path[i] == '/')
			continue;
		if(isprint(path[i]) != 0 && path[i] != POD_PATH_SEPARATOR)
			continue;
		return false;
	}
	return true;
}

bool pod_path_is_pod(pod_path_t path)
{
	if(path == NULL)
		return false;

	for(int i = 0; i < strlen(path); i++)
	{
		if(path[i] == POD_PATH_SEPARATOR)
			continue;
		if(isprint(path[i]) != 0)
			continue;

		return false;
	}

	return true;
}

bool pod_path_is_win32(pod_path_t path)
{
	if(isalpha(path[0]) == 0 || path[1] != ':')
		return false;

	for(int i = 0; i < strlen(path); i++)
	{
		if(path[i] == POD_PATH_SEPARATOR)
			continue;
		if(isprint(path[i]) != 0)
			continue;

		return false;
	}

	return true;
}

pod_path_t pod_path_append_win32(pod_path_t a, pod_path_t b)
{
	pod_number_t size_a = strlen(a);
	pod_number_t size_b = strlen(b);
	pod_number_t size = size_a + size_b + 2;
	pod_path_t path = calloc(1, size);
	pod_char_t separator = POD_PATH_SEPARATOR;

	if(!strncpy(path, a, size_a))
		return NULL;

	pod_number_t start = size_a;

	if(path[start - 1] == separator && b[0] == separator)
		start--;
	else if(path[start - 1] != separator && b[0] != separator)
		path[start++] = separator;

	if(!strncpy(&path[start], b, size_b))
		return NULL;

	return path;
}




pod_path_t pod_path_append_posix(pod_path_t a, pod_path_t b)
{
	pod_number_t size_a = strlen(a);
	pod_number_t size_b = strlen(b);
	pod_number_t size = size_a + size_b + 2;
	pod_path_t path = calloc(1, size);
	pod_char_t separator = '/';

	if(!pod_path_is_posix(a) || !pod_path_is_posix(b))
		return NULL;

	if(!strncpy(path, a, size_a))
		return NULL;

	pod_number_t start = size_a;

	if(path[start - 1] == separator && b[0] == separator)
		start--;
	else if(path[start - 1] != separator && b[0] != separator)
		path[start++] = separator;

	if(!strncpy(&path[start], b, size_b))
		return NULL;

	return path;
}

