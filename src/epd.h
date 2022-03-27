#ifndef _EPD_H
#define _EPD_H

#include "pod_common.h"
#include "pod_zip.h"

/* Extended POD file format (EPD) */

enum pod_audit_entry_epd_action_t
{
	EPD_AUDIT_ACTION_ADD    = 0,
	EPD_AUDIT_ACTION_REMOVE = 1,
	EPD_AUDIT_ACTION_CHANGE = 2,
	EPD_AUDIT_ACTION_SIZE   = 3,
};

typedef enum pod_audit_entry_epd_action_t pod_audit_entry_epd_action_t;

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
typedef struct pod_entry_epd_s {
	pod_char_t name[POD_DIR_ENTRY_EPD_FILENAME_SIZE];
	pod_number_t size;
	pod_number_t offset;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod_entry_epd_t;

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

/* EPD zip entry */
typedef struct pod_zip_entry_epd_s
{
	zip_source_t *src;
	zip_t *za;
	zip_error_t error;
	pod_entry_epd_t* entry;
	pod_char_t* name;
} pod_zip_entry_epd_t;

/* EPD file data structure */
typedef struct pod_file_epd_s
{
	pod_header_epd_t* header;
	pod_entry_epd_t* entries; /* header.file_count */
	pod_byte_t* entry_data;
	pod_audit_entry_epd_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_byte_t* data;
	pod_number_t* gap_sizes;
	pod_size_t entry_data_size;
	pod_size_t audit_data_size;

	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data_start;
	pod_zip_entry_epd_t* zip_entries;
	/* end of not serialized content */
} pod_file_epd_t;

bool pod_is_epd(char* ident);

uint32_t pod_crc_epd(pod_file_epd_t* file);
uint32_t pod_crc_epd_entry(pod_file_epd_t* file, pod_number_t entry_index);
uint32_t pod_crc_epd_audit(pod_file_epd_t* file, pod_number_t audit_index);
pod_bool_t pod_file_epd_update_sizes(pod_file_epd_t* pod_file);
pod_file_epd_t* pod_file_epd_create(pod_string_t filename);
pod_file_epd_t* pod_file_epd_delete(pod_file_epd_t* podfile);
pod_checksum_t  pod_file_epd_chksum(pod_file_epd_t* podfile);
bool pod_file_epd_print(pod_file_epd_t* podfile);
bool pod_file_epd_write(pod_file_epd_t* pod_file, pod_string_t filename);
bool pod_audit_entry_epd_print(pod_audit_entry_epd_t* audit);
bool pod_file_epd_extract(pod_file_epd_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif
