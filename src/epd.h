#ifndef _EPD_H
#define _EPD_H

#include "pod_common.h"

/* EPD header data structure */
typedef struct pod_header_epd_s
{
	pod_char_t ident[POD_HEADER_IDENT_SIZE];
	pod_char_t comment[POD_HEADER_EPD_COMMENT_SIZE];
	pod_number_t file_count;
	pod_number_t version;
	pod_number_t checksum;
} pod_header_epd_t;

/* EPD entry data structure */
typedef struct pod_entry_pod_s {
	pod_char_t name[POD_DIR_ENTRY_EPD_FILENAME_SIZE];
	pod_number_t size;
	pod_number_t offset;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod_entry_epd_t;

bool pod_is_epd(char* ident);

/* EPD audit trail entry data structure */
typedef struct pod_audit_entry_epd_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_epd_t;

/* EPD file data structure */
typedef struct pod_file_epd_s
{
	pod_header_epd_t* header;
	pod_entry_epd_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_byte_t* entry_data;
	pod_audit_entry_epd_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_size_t path_data_size;
	pod_size_t entry_data_size;

	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data;
	/* end of not serialized content */
} pod_file_epd_t;

#endif
