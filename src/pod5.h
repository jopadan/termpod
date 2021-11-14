#ifndef _POD5_H
#define _POD5_H

#include "pod_common.h"
#include <zip.h>

enum pod_audit_entry_pod5_action_t
{
	POD5_AUDIT_ACTION_ADD    = 0,
	POD5_AUDIT_ACTION_REMOVE = 1,
	POD5_AUDIT_ACTION_CHANGE = 2,
	POD5_AUDIT_ACTION_SIZE   = 3,
};
typedef enum pod_audit_entry_pod5_action_t pod_audit_entry_pod5_action_t;

/* POD5 header data structure */
typedef struct pod_header_pod5_s
{
	pod_char_t ident[POD_HEADER_IDENT_SIZE];
	pod_number_t checksum;
	pod_char_t comment[POD_HEADER_COMMENT_SIZE];
	pod_number_t file_count;
	pod_number_t audit_file_count;
	pod_number_t revision;
	pod_number_t priority;
	pod_char_t author[POD_HEADER_AUTHOR_SIZE];
	pod_char_t copyright[POD_HEADER_COPYRIGHT_SIZE];
	pod_number_t index_offset;
	pod_number_t unknown10c;
	pod_number_t size_index;
	pod_signed_number_t flag0; /* numerical boolean: -1 / 0 */
	pod_signed_number_t flag1; /* numerical boolean: -1 / 0 */
	pod_signed_number_t pad11c;
	pod_char_t next_archive[POD_HEADER_NEXT_ARCHIVE_SIZE];
} pod_header_pod5_t;

/* POD5 entry data structure */
typedef struct pod_entry_pod5_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_number_t uncompressed;
	pod_number_t compression_level;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod_entry_pod5_t;

/* POD5 audit trail entry data structure */
typedef struct pod_audit_entry_pod5_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod5_t;

/* POD5 zip entry data structure */
typedef struct pod_zip_entry_pod5_s
{
	zip_source_t *src;
	zip_t *za;
	zip_error_t error;
	pod_entry_pod5_t* entry;
	pod_char_t* name;
} pod_zip_entry_pod5_t;

/* POD5 file data structure */
typedef struct pod_file_pod5_s
{
	pod_header_pod5_t* header;
	pod_byte_t* entry_data;
	pod_entry_pod5_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_audit_entry_pod5_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_byte_t* data;
	pod_number_t* gap_sizes;
	pod_size_t path_data_size;
	pod_size_t entry_data_size;
	pod_size_t audit_data_size;
	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data_start;
	pod_zip_entry_pod5_t* zip_entries;
	/* end of not serialized content */
} pod_file_pod5_t;

bool pod_is_pod5(char* ident);

uint32_t pod_crc_pod5(pod_file_pod5_t* file);
uint32_t pod_crc_pod5_entry(pod_file_pod5_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod5_audit(pod_file_pod5_t* file, pod_number_t audit_index);
pod_bool_t pod_file_pod5_update_sizes(pod_file_pod5_t* pod_file);
pod_file_pod5_t* pod_file_pod5_create(pod_string_t filename);
bool pod_file_pod5_destroy(pod_file_pod5_t* podfile);
bool pod_file_pod5_print(pod_file_pod5_t* podfile);
bool pod_file_pod5_write(pod_file_pod5_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod5_print(pod_audit_entry_pod5_t* audit);
bool pod_file_pod5_extract(pod_file_pod5_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif
