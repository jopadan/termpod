#ifndef _POD2_H
#define _POD2_H

enum pod_audit_entry_pod2_action_t
{
	POD2_AUDIT_ACTION_ADD    = 0,
	POD2_AUDIT_ACTION_REMOVE = 1,
	POD2_AUDIT_ACTION_CHANGE = 2,
	POD2_AUDIT_ACTION_SIZE   = 3,
};
typedef enum pod_audit_entry_pod2_action_t pod_audit_entry_pod2_action_t;

extern const char pod_audit_action_string[POD2_AUDIT_ACTION_SIZE][8];

/* POD2 header data structure */
typedef struct pod_header_pod2_s
{
	pod_char_t ident[POD_IDENT_SIZE];
	pod_number_t checksum;
	pod_char_t comment[POD_COMMENT_SIZE];
	pod_number_t file_count;
	pod_number_t audit_file_count;
} pod_header_pod2_t;

/* POD2 entry data structure */
typedef struct pod_entry_pod2_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_time_t timestamp;
	pod_number_t checksum;
} pod_entry_pod2_t;

/* POD2 audit trail entry data structure */
typedef struct pod_audit_entry_pod2_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod2_t;

/* POD2 file data structure */
typedef struct pod_file_pod2_s
{
	pod_header_pod2_t* header;
	pod_entry_pod2_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_byte_t* entry_data;
	pod_audit_entry_pod2_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_size_t path_data_size;
	pod_size_t entry_data_size;

	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data;
	/* end of not serialized content */
} pod_file_pod2_t;

bool is_pod2(char* ident);
uint32_t pod_crc(pod_byte_t* data, pod_size_t count);
uint32_t pod_crc_pod2(pod_file_pod2_t* file);
uint32_t pod_crc_pod2_entry(pod_file_pod2_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod2_audit(pod_file_pod2_t* file, pod_number_t audit_index);
pod_file_pod2_t* pod_file_pod2_create(pod_string_t filename);
bool pod_file_pod2_destroy(pod_file_pod2_t* podfile);
bool pod_file_pod2_print(pod_file_pod2_t* podfile);
bool pod_file_pod2_write(pod_file_pod2_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod2_print(pod_audit_entry_pod2_t* audit);
bool pod_file_pod2_extract(pod_file_pod2_t* pod_file, pod_string_t dst);
#endif
