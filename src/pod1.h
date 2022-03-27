#ifndef _POD1_H
#define _POD1_H

/* POD1 header data structure */
typedef struct pod_header_pod1_s
{
	pod_number_t file_count;
	pod_char_t comment[POD_HEADER_COMMENT_SIZE];
} pod_header_pod1_t;

/* POD1 entry data structure */
typedef struct pod_entry_pod1_s {
	pod_char_t name[POD_DIR_ENTRY_POD1_FILENAME_SIZE];
	pod_number_t size;
	pod_number_t offset;
} pod_entry_pod1_t;

bool pod_is_pod1(char* ident);

enum pod_audit_entry_pod1_action_t
{
	POD1_AUDIT_ACTION_ADD    = 0,
	POD1_AUDIT_ACTION_REMOVE = 1,
	POD1_AUDIT_ACTION_CHANGE = 2,
	POD1_AUDIT_ACTION_SIZE   = 3,
};


typedef enum pod_audit_entry_pod1_action_t pod_audit_entry_pod1_action_t;

extern const char pod_audit_action_string[POD1_AUDIT_ACTION_SIZE][8];

/* POD1 audit trail entry data structure */
typedef struct pod_audit_entry_pod1_s
{
	pod_char_t user[POD_STRING_32];
	pod_time_t timestamp;
	pod_number_t action;
	pod_char_t path[POD_STRING_256];
	pod_time_t old_timestamp;
	pod_number_t old_size;
	pod_time_t new_timestamp;
	pod_number_t new_size;
} pod_audit_entry_pod1_t;

/* POD1 file data structure */
typedef struct pod_file_pod_s
{
	pod_header_pod1_t* header;
	pod_entry_pod1_t* entries; /* header.file_count */
	pod_byte_t* entry_data;
	/* not serialized content */
	pod_size_t entry_data_size;
	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data;
	/* end of not serialized content */
} pod_file_pod1_t;

bool pod_is_pod1(char* ident);
uint32_t pod_crc_pod1(pod_file_pod1_t* file);
uint32_t pod_crc_pod1_entry(pod_file_pod1_t* file, pod_number_t entry_index);
uint32_t pod_crc_pod1_audit(pod_file_pod1_t* file, pod_number_t audit_index);
pod_file_pod1_t* pod_file_pod1_create(pod_string_t filename);
pod_file_pod1_t* pod_file_pod1_delete(pod_file_pod1_t* podfile);
pod_checksum_t   pod_file_pod1_chksum(pod_file_pod1_t* podfile);
bool pod_file_pod1_print(pod_file_pod1_t* podfile);
bool pod_file_pod1_write(pod_file_pod1_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod1_print(pod_audit_entry_pod1_t* audit);
bool pod_file_pod1_extract(pod_file_pod1_t* pod_file, pod_string_t dst, pod_bool_t absolute);
#endif
