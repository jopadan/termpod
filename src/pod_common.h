#ifndef _POD_COMMON_H
#define _POD_COMMON_H

#ifndef __ATFILE_VISIBLE
#define __ATFILE_VISIBLE 1
#endif
#ifndef __USE_GNU
#define __USE_GNU 1
#endif
#ifdef __WIN32__
#ifdef __MSYS__
#include <sys/cygwin.h>
#elif defined __CYGWIN__
#include <sys/cygwin.h>
#endif
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <libgen.h> 
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ccitt32_crc.h"
#include <ctype.h>
#include <bsd/timeconv.h>

/* main variable type sizes of POD file formats                                                            */
typedef uint32_t                             pod_number_t;
typedef size_t                               pod_size_t;
typedef ssize_t                              pod_ssize_t;
typedef off_t                                pod_off_t;
typedef uint8_t                              pod_byte_t;
typedef int8_t                               pod_char_t;
typedef wchar_t                              pod_wchar_t;
typedef pod_char_t*                          pod_string_t;
typedef wchar_t*                             pod_wchar_string_t;
typedef int32_t                              pod_time32_t;
typedef int64_t                              pod_time64_t;
typedef pod_time32_t                         pod_time_t;
typedef pod_char_t*                          pod_path_t;
typedef bool                                 pod_bool_t;
#define POD_NUMBER_SIZE                      sizeof(pod_number_t)          /* length of a numerical entry    */
#define POD_BYTE_SIZE                        sizeof(pod_byte_t)            /* length of a byte entry         */
#define POD_CHAR_SIZE                        sizeof(pod_char_t)            /* length of a character entry    */
#define POD_STRING_SIZE                      sizeof(pod_string_t)          /* length string char* entry      */
#define POD_TIME_SIZE                        sizeof(pod_time_t)            /* length of a time entry         */
#define POD_HEADER_CHECKSUM_DEFAULT          0x44424247                    /* default checksum of POD file   */
#define POD_ENTRY_CHECKSUM_DEFAULT           0x20444542                    /* default checksum of POD entry  */
#define POD_ENTRY_TIMESTAMP_DEFAULT          0x42494720                    /* default timestamp of POD entry */
#define POD_HEADER_UNKNOWN10C_DEFAULT        0x58585858			   /* default value of unknown10c    */
#define POD_CHECKSUM_DEFAULT                 0xFFFFFFFF                    /* default seed for CCIT32-CRC    */
#define POD_PATH_SEPARATOR                   '\\'                          /* default path separator         */
#define POD_PATH_NULL                        '\0'
#define POD_SYSTEM_PATH_SIZE                 1024                          /* default system path length     */
#define POD_UMASK                            0755                          /* default UMASK privileges       */
pod_string_t pod_ctime(pod_time_t* time32);

enum pod_string_size_t
{
	POD_STRING_4=4, POD_STRING_32=32, POD_STRING_48=48, POD_STRING_64=64, POD_STRING_80=80,
	POD_STRING_128=128, POD_STRING_256=256, POD_STRING_264=264,
	POD_STRING_SIZE_SIZE=8,
};
typedef enum pod_string_size_t pod_string_size_t;

#define POD_COMMENT_SIZE                     POD_STRING_80                 /* comment length of POD format   */
#define EPD_COMMENT_SIZE                     (POD_BYTE_SIZE + 1)           /* comment length of EPD format   */

enum pod_ident_type_t
{
	POD1 = 0, POD2, POD3, 
	POD4, POD5, POD6, 
	EPD, POD_IDENT_TYPE_SIZE, 
};
typedef enum pod_ident_type_t pod_ident_type_t;

#define POD_IDENT_SIZE                       POD_STRING_4             /* file magic ident length        */
#define POD_IDENT_TYPE_SIZE                  (EPD + 1)                /* number of POD format types     */



/* pod_header_t element sizes */
#define POD_HEADER_NUMBER_SIZE               POD_NUMBER_SIZE
#define POD_HEADER_COMMENT_SIZE              POD_COMMENT_SIZE
#define POD_HEADER_EPD_COMMENT_SIZE          EPD_COMMENT_SIZE
#define POD_HEADER_IDENT_SIZE                POD_IDENT_SIZE
#define POD_HEADER_NEXT_ARCHIVE_SIZE         POD_HEADER_COMMENT_SIZE
#define POD_HEADER_AUTHOR_SIZE               POD_HEADER_COMMENT_SIZE
#define POD_HEADER_COPYRIGHT_SIZE            POD_HEADER_COMMENT_SIZE
#define POD_HEADER_CHECKSUM_SIZE             POD_HEADER_NUMBER_SIZE
#define POD_HEADER_FILE_COUNT_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_AUDIT_FILE_COUNT_SIZE     POD_HEADER_NUMBER_SIZE
#define POD_HEADER_REVISION_SIZE             POD_HEADER_NUMBER_SIZE
#define POD_HEADER_VERSION_SIZE              POD_HEADER_NUMBER_SIZE
#define POD_HEADER_PRIORITY_SIZE             POD_HEADER_NUMBER_SIZE
#define POD_HEADER_INDEX_OFFSET_SIZE         POD_HEADER_NUMBER_SIZE
#define POD_HEADER_SIZE_INDEX_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_NAME_COUNT_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_UNKNOWN10C_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_UNKNOWN114_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_UNKNOWN118_SIZE           POD_HEADER_NUMBER_SIZE
#define POD_HEADER_UNKNOWN11C_SIZE           POD_HEADER_NUMBER_SIZE

#define POD_DIR_ENTRY_FILENAME_SIZE          POD_STRING_256
#define POD_DIR_ENTRY_NUMBER_SIZE            POD_NUMBER_SIZE
#define POD_DIR_ENTRY_SIZE_SIZE              POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_OFFSET_SIZE            POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_PATH_OFFSET_SIZE       POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_UNCOMPRESSED_SIZE      POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_COMPRESSION_LEVEL_SIZE POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_TIMESTAMP_SIZE         POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_CHECKSUM_SIZE          POD_DIR_ENTRY_NUMBER_SIZE
#define POD_DIR_ENTRY_ZERO_SIZE              POD_DIR_ENTRY_NUMBER_SIZE

/* audit entry sizes for POD2 */
#define POD_AUDIT_ENTRY_USER_SIZE            POD_STRING_32
#define POD_AUDIT_ENTRY_TIMESTAMP_SIZE       POD_TIME_SIZE
#define POD_AUDIT_ENTRY_ACTION_SIZE          POD_NUMBER_SIZE
#define POD_AUDIT_ENTRY_PATH_SIZE            POD_STRING_256
#define POD_AUDIT_ENTRY_DATA_SIZE            POD_BYTE_SIZE * 16


/* POD1 header sizes */
#define POD_HEADER_POD1_SIZE (POD_HEADER_FILE_COUNT_SIZE + \
                              POD_HEADER_COMMENT_SIZE)

#define POD_DIR_ENTRY_POD1_SIZE (POD_DIR_ENTRY_POD1_FILENAME_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
				 POD_DIR_ENTRY_OFFSET_SIZE)

#define POD_DIR_ENTRY_POD1_FILENAME_SIZE     POD_STRING_32

/* POD2 header sizes */
#define POD_HEADER_POD2_SIZE (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_CHECKSUM_SIZE + \
			      POD_HEADER_COMMENT_SIZE + \
			      POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_AUDIT_FILE_COUNT_SIZE)

#define POD_DIR_ENTRY_POD2_SIZE (POD_DIR_ENTRY_PATH_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
				 POD_DIR_ENTRY_OFFSET_SIZE + \
				 POD_DIR_ENTRY_TIMESTAMP_SIZE + \
				 POD_DIR_ENTRY_CHECKSUM_SIZE)

#define POD_DIR_ENTRY_POD2_FILENAME_SIZE     POD_DIR_ENTRY_FILENAME_SIZE
#define POD_AUDIT_ENTRY_POD2_SIZE (POD_AUDIT_ENTRY_USER_SIZE + \
				   POD_AUDIT_ENTRY_TIMESTAMP_SIZE + \
				   POD_AUDIT_ENTRY_ACTION_SIZE + \
				   POD_AUDIT_ENTRY_PATH_SIZE + \
				   POD_AUDIT_ENTRY_DATA_SIZE)
		            
/* POD3 header sizes*/
#define POD_HEADER_POD3_SIZE (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_CHECKSUM_SIZE + \
			      POD_HEADER_COMMENT_SIZE + \
			      POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_AUDIT_FILE_COUNT_SIZE + \
			      POD_HEADER_REVISION_SIZE + \
			      POD_HEADER_PRIORITY_SIZE + \
			      POD_HEADER_AUTHOR_SIZE + \
			      POD_HEADER_COPYRIGHT_SIZE + \
			      POD_HEADER_INDEX_OFFSET_SIZE + \
			      POD_HEADER_UNKNOWN10C_SIZE + \
			      POD_HEADER_SIZE_INDEX_SIZE + \
			      POD_HEADER_UNKNOWN114_SIZE + \
			      POD_HEADER_UNKNOWN118_SIZE + \
			      POD_HEADER_UNKNOWN11C_SIZE)

#define POD_DIR_ENTRY_POD3_SIZE (POD_DIR_ENTRY_PATH_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
				 POD_DIR_ENTRY_OFFSET_SIZE + \
				 POD_DIR_ENTRY_TIMESTAMP_SIZE + \
				 POD_DIR_ENTRY_CHECKSUM_SIZE + \
				 POD_DIR_ENTRY_FILENAME_SIZE)

#define POD_DIR_ENTRY_POD3_FILENAME_SIZE     POD_DIR_ENTRY_FILENAME_SIZE

/* POD4 header sizes */
#define POD_HEADER_POD4_SIZE (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_CHECKSUM_SIZE + \
			      POD_HEADER_COMMENT_SIZE + \
			      POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_AUDIT_FILE_COUNT_SIZE + \
			      POD_HEADER_REVISION_SIZE + \
			      POD_HEADER_PRIORITY_SIZE + \
			      POD_HEADER_AUTHOR_SIZE + \
			      POD_HEADER_COPYRIGHT_SIZE + \
			      POD_HEADER_INDEX_OFFSET_SIZE + \
			      POD_HEADER_UNKNOWN10C_SIZE + \
			      POD_HEADER_SIZE_INDEX_SIZE + \
			      POD_HEADER_UNKNOWN114_SIZE + \
			      POD_HEADER_UNKNOWN118_SIZE + \
			      POD_HEADER_UNKNOWN11C_SIZE)

#define POD_DIR_ENTRY_POD4_SIZE (POD_DIR_ENTRY_PATH_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
				 POD_DIR_ENTRY_OFFSET_SIZE + \
				 POD_DIR_ENTRY_UNCOMPRESSED_SIZE + \
				 POD_DIR_ENTRY_COMPRESSION_LEVEL_SIZE + \
				 POD_DIR_ENTRY_TIMESTAMP_SIZE + \
				 POD_DIR_ENTRY_CHECKSUM_SIZE + \
				 POD_DIR_ENTRY_FILENAME_SIZE)

#define POD_DIR_ENTRY_POD4_FILENAME_SIZE     POD_DIR_ENTRY_FILENAME_SIZE

/* POD5 header sizes */
#define POD_HEADER_POD5_SIZE (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_CHECKSUM_SIZE + \
			      POD_HEADER_COMMENT_SIZE + \
			      POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_AUDIT_FILE_COUNT_SIZE + \
			      POD_HEADER_REVISION_SIZE + \
			      POD_HEADER_PRIORITY_SIZE + \
			      POD_HEADER_AUTHOR_SIZE + \
			      POD_HEADER_COPYRIGHT_SIZE + \
			      POD_HEADER_INDEX_OFFSET_SIZE + \
			      POD_HEADER_UNKNOWN10C_SIZE + \
			      POD_HEADER_SIZE_INDEX_SIZE + \
			      POD_HEADER_UNKNOWN114_SIZE + \
			      POD_HEADER_UNKNOWN118_SIZE + \
			      POD_HEADER_UNKNOWN11C_SIZE + \
			      POD_HEADER_NEXT_ARCHIVE_SIZE)

#define POD_DIR_ENTRY_POD5_SIZE (POD_DIR_ENTRY_PATH_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
				 POD_DIR_ENTRY_OFFSET_SIZE + \
				 POD_DIR_ENTRY_UNCOMPRESSED_SIZE + \
				 POD_DIR_ENTRY_COMPRESSION_LEVEL_SIZE + \
				 POD_DIR_ENTRY_TIMESTAMP_SIZE + \
				 POD_DIR_ENTRY_CHECKSUM_SIZE + \
				 POD_DIR_ENTRY_FILENAME_SIZE)

#define POD_DIR_ENTRY_POD5_FILENAME_SIZE     POD_DIR_ENTRY_FILENAME_SIZE

/* POD6 header sizes */
#define POD_HEADER_POD6_SIZE (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_VERSION_SIZE + \
			      POD_HEADER_INDEX_OFFSET_SIZE + \
			      POD_HEADER_SIZE_INDEX_SIZE)

#define POD_DIR_ENTRY_POD6_SIZE (POD_DIR_ENTRY_PATH_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_SIZE_SIZE + \
                                 POD_DIR_ENTRY_OFFSET_SIZE + \
                                 POD_DIR_ENTRY_UNCOMPRESSED_SIZE + \
                                 POD_DIR_ENTRY_COMPRESSION_LEVEL_SIZE + \
                                 POD_DIR_ENTRY_ZERO_SIZE)

#define POD_DIR_ENTRY_POD6_FILENAME_SIZE     POD_DIR_ENTRY_FILENAME_SIZE

/* EPD header sizes */
#define POD_HEADER_EPD_SIZE  (POD_HEADER_IDENT_SIZE + \
                              POD_HEADER_EPD_COMMENT_SIZE + \
			      POD_HEADER_FILE_COUNT_SIZE + \
			      POD_HEADER_VERSION_SIZE + \
			      POD_HEADER_CHECKSUM_SIZE)

#define POD_DIR_ENTRY_EPD_SIZE (POD_DIR_ENTRY_EPD_FILENAME_SIZE + \
                                POD_DIR_ENTRY_SIZE_SIZE + \
				POD_DIR_ENTRY_OFFSET_SIZE + \
				POD_DIR_ENTRY_TIMESTAMP_SIZE + \
				POD_DIR_ENTRY_CHECKSUM_SIZE) 

#define POD_DIR_ENTRY_EPD_FILENAME_SIZE      POD_STRING_64


extern const char POD_IDENT[ POD_IDENT_TYPE_SIZE ][ POD_IDENT_SIZE + 1 ];


extern pod_ident_type_t pod_type(char* ident);
extern bool is_pod(char* ident);
extern pod_string_t pod_type_to_file_ext(int pod_type);
extern const char* pod_type_str(pod_ident_type_t type);
extern FILE* pod_fopen_mkdir(pod_string_t path, char* mode);
extern bool pod_rec_mkdir(pod_string_t path, char separator);
extern bool pod_directory_create(pod_string_t path, char separator);

/* system path functions */
extern pod_path_t pod_path_system_home();
extern pod_path_t pod_path_system_root();
extern pod_char_t pod_path_system_drive();
extern pod_path_t pod_path_posix_to_win32(pod_path_t src, pod_char_t separator, pod_bool_t absolute, pod_char_t drive);
extern pod_path_t pod_path_append_posix(pod_path_t a, pod_path_t b);
extern pod_path_t pod_path_append_win32(pod_path_t a, pod_path_t b);
extern bool pod_path_is_win32(pod_path_t path);
extern bool pod_path_is_posix(pod_path_t path);

/* time_t convert functions */
extern time_t pod_time32_to_time(pod_time32_t t32);
extern time_t pod_time64_to_time(pod_time64_t t64);
extern pod_time32_t pod_time_to_time32(time_t t);
extern pod_time64_t pod_time_to_time64(time_t t);

extern const ssize_t POD_DIR_ENTRY_SIZE[POD_IDENT_TYPE_SIZE];
extern const ssize_t POD_HEADER_SIZE[POD_IDENT_TYPE_SIZE];

typedef struct pod_dir_entry_s {
	pod_char_t* filename[POD_IDENT_TYPE_SIZE];
	pod_number_t file_path_offset;
	pod_number_t file_size;
	pod_number_t file_offset;
	pod_number_t file_uncompressed_size;
	pod_number_t file_compression_level;
	pod_time_t file_timestamp;
	pod_number_t file_checksum;
} pod_dir_entry_t;

#endif
