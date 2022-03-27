#ifndef _POD3_H
#define _POD3_H

#include "pod_common.h"
#include <zip.h>

enum pod_audit_entry_pod3_action_t
{
	POD3_AUDIT_ACTION_ADD    = 0,
	POD3_AUDIT_ACTION_REMOVE = 1,
	POD3_AUDIT_ACTION_CHANGE = 2,
	POD3_AUDIT_ACTION_SIZE   = 3,
};

typedef enum pod_audit_entry_pod3_action_t pod_audit_entry_pod3_action_t;

/* POD3 header data structure */
typedef struct pod_header_pod3_s
{
	pod_char_t ident[POD_HEADER_IDENT_SIZE];
	pod_number_t checksum; /* of data after checksum */
	pod_char_t comment[POD_HEADER_COMMENT_SIZE]; 
	pod_number_t file_count;
	pod_number_t audit_file_count;
	pod_number_t revision;
	pod_number_t priority;
	pod_char_t author[POD_HEADER_AUTHOR_SIZE];
	pod_char_t copyright[POD_HEADER_COPYRIGHT_SIZE];
	pod_number_t index_offset; /* 0x108 */
	pod_number_t pad10c;       /* 0x10c */
	pod_number_t size_index;   /* 0x110 */
	pod_signed_number_t flag0;  /* -1 / 0  */ /* 0x114 */
	pod_signed_number_t flag1;  /* -1 / 0 */ /* 0x118 */
	pod_signed_number_t pad11c; /* 0xFFFFFFFF 0xD4009345 0x64B5C42D 0xA1FE0F74 */
	pod_number_t pad120; /* 0x120 */
	pod_number_t pad124; /* 0x124 */
} pod_header_pod3_t;

/* POD3 entry data structure */
typedef struct pod_entry_pod3_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod_entry_pod3_t;

/* POD3 audit trail entry data structure */
typedef struct pod_audit_entry_pod3_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod3_t;

/* POD3 zip entry data structure */
typedef struct pod_zip_entry_pod3_s
{
	zip_source_t *src;
	zip_t *za;
	zip_error_t error;
	pod_entry_pod3_t* entry;
	pod_char_t* name;
} pod_zip_entry_pod3_t;

/* POD3 file data structure */
typedef struct pod_file_pod3_s
{
	pod_header_pod3_t* header;
	pod_entry_pod3_t* entries; /* header.file_count */
	pod_byte_t* entry_data;
	pod_char_t* path_data;
	pod_audit_entry_pod3_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_byte_t* data;
	pod_number_t* gap_sizes;
	pod_size_t path_data_size;
	pod_size_t entry_data_size;
	pod_size_t audit_data_size;
	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_number_t data_offset;
	pod_byte_t* data_start;
	pod_zip_entry_pod3_t* zip_entries;
	/* end of not serialized content */
} pod_file_pod3_t;
bool pod_is_pod3(char* ident);

uint32_t pod_crc(pod_byte_t* data, pod_size_t count);
uint32_t pod_crc_pod3(pod_file_pod3_t* file);
uint32_t pod_crc_pod3_entry(pod_file_pod3_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod3_audit(pod_file_pod3_t* file, pod_number_t audit_index);
pod_bool_t pod_file_pod3_update_sizes(pod_file_pod3_t* pod_file);
pod_checksum_t   pod_file_pod3_chksum(pod_file_pod3_t* podfile);
pod_file_pod3_t* pod_file_pod3_create(pod_string_t filename);
pod_file_pod3_t* pod_file_pod3_delete(pod_file_pod3_t* podfile);
bool pod_file_pod3_print(pod_file_pod3_t* podfile);
bool pod_file_pod3_write(pod_file_pod3_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod3_print(pod_audit_entry_pod3_t* audit);
bool pod_file_pod3_extract(pod_file_pod3_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif
