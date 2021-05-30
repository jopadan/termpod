#ifndef _POD6_H
#define _POD6_H

#include "pod_common.h"
#include <zip.h>


enum pod_audit_entry_pod6_action_t
{
	POD6_AUDIT_ACTION_ADD    = 0,
	POD6_AUDIT_ACTION_REMOVE = 1,
	POD6_AUDIT_ACTION_CHANGE = 2,
	POD6_AUDIT_ACTION_SIZE   = 3,
};

typedef enum pod_audit_entry_pod6_action_t pod_audit_entry_pod6_action_t;


/* POD6 header data structure */
typedef struct pod_header_pod6_s
{
	pod_char_t ident[POD_HEADER_IDENT_SIZE];
	pod_number_t file_count;
	pod_number_t version;
	pod_number_t index_offset;
	pod_number_t size_index;
} pod_header_pod6_t;

/* POD6 entry data structure */
typedef struct pod_entry_pod6_s {
	pod_number_t path_offset; /* names = &(SEEK_SET + name_offset + header.names_offset] */
	pod_number_t size;
	pod_number_t offset;
	pod_number_t uncompressed;
	pod_number_t compression_level;
	pod_number_t zero;
} pod_entry_pod6_t;

/* POD6 audit trail entry data structure */
typedef struct pod_audit_entry_pod6_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod6_t;

/* POD6 zip entry */
typedef struct pod_zip_entry_pod6_s
{
	zip_source_t *src;
	zip_t *za;
	zip_error_t error;
	pod_entry_pod6_t* entry;
	pod_char_t* name;
} pod_zip_entry_pod6_t;

/* POD6 file data structure */
typedef struct pod_file_pod6_s
{
	pod_header_pod6_t* header;
	pod_byte_t* entry_data;
	pod_entry_pod6_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_audit_entry_pod6_t* audit_trail; /* header.audit_file_count */
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
	pod_zip_entry_pod6_t* zip_entries;
	/* end of not serialized content */
} pod_file_pod6_t;

bool pod_is_pod6(char* ident);

uint32_t pod_crc_pod6(pod_file_pod6_t* file);
uint32_t pod_crc_pod6_entry(pod_file_pod6_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod6_audit(pod_file_pod6_t* file, pod_number_t audit_index);
pod_bool_t pod_file_pod6_update_sizes(pod_file_pod6_t* pod_file);
pod_file_pod6_t* pod_file_pod6_create(pod_string_t filename);
bool pod_file_pod6_destroy(pod_file_pod6_t* podfile);
bool pod_file_pod6_print(pod_file_pod6_t* podfile);
bool pod_file_pod6_write(pod_file_pod6_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod6_print(pod_audit_entry_pod6_t* audit);
bool pod_file_pod6_extract(pod_file_pod6_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif
