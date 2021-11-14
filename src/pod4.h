#ifndef _POD4_H
#define _POD4_H

#include "pod_common.h"
#include <zip.h>

enum pod_audit_entry_pod4_action_t
{
	POD4_AUDIT_ACTION_ADD    = 0,
	POD4_AUDIT_ACTION_REMOVE = 1,
	POD4_AUDIT_ACTION_CHANGE = 2,
	POD4_AUDIT_ACTION_SIZE   = 3,
};
typedef enum pod_audit_entry_pod4_action_t pod_audit_entry_pod4_action_t;


/* POD4 header data structure */
typedef struct pod_header_pod4_s
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
	pod_number_t pad10c;       /* 0x10c */
	pod_number_t size_index;   /* 0x110 */
	pod_number_t flag0;  /* numerical boolean: -1 / 0 */ /* 0x114 */
	pod_number_t flag1;  /* numerical boolean: -1 / 0 */ /* 0x118 */
	pod_number_t pad11c; /* 0xFFFFFFFF 0xD4009345 0x64B5C42D 0xA1FE0F74 */
	pod_number_t pad120; /* 0x120 */
	pod_number_t pad124; /* 0x124 */
} pod_header_pod4_t;

/* POD4 entry data structure */
typedef struct pod_entry_pod4_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_number_t uncompressed;
	pod_number_t compression_level;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod_entry_pod4_t;

/* POD4 audit trail entry data structure */
typedef struct pod_audit_entry_pod4_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod4_t;

/* POD4 zip entry */
typedef struct pod_zip_entry_pod4_s
{
	zip_source_t *src;
	zip_t *za;
	zip_error_t error;
	pod_entry_pod4_t* entry;
	pod_char_t* name;
} pod_zip_entry_pod4_t;

/* POD4 file data structure */
typedef struct pod_file_pod4_s
{
	pod_header_pod4_t* header;
	pod_entry_pod4_t* entries; /* header.file_count */
	pod_byte_t* entry_data;
	pod_char_t* path_data;
	pod_audit_entry_pod4_t* audit_trail; /* header.audit_file_count */
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
	pod_zip_entry_pod4_t* zip_entries;
	/* end of not serialized content */
} pod_file_pod4_t;


bool pod_is_pod4(char* ident);
uint32_t pod_crc_pod4(pod_file_pod4_t* file);
uint32_t pod_crc_pod4_entry(pod_file_pod4_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod4_audit(pod_file_pod4_t* file, pod_number_t audit_index);
pod_bool_t pod_file_pod4_update_sizes(pod_file_pod4_t* pod_file);
pod_file_pod4_t* pod_file_pod4_create(pod_string_t filename);
bool pod_file_pod4_destroy(pod_file_pod4_t* podfile);
bool pod_file_pod4_print(pod_file_pod4_t* podfile);
bool pod_file_pod4_write(pod_file_pod4_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod4_print(pod_audit_entry_pod4_t* audit);
bool pod_file_pod4_extract(pod_file_pod4_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif

