#ifndef _POD3_H
#define _POD3_H

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
	pod_number_t index_offset;
	pod_number_t unknown10c;
	pod_number_t size_index;
	pod_number_t number_min; /* 0x00000000 */
	pod_number_t number_max; /* 0xFFFFFFFF */
	pod_number_t unknown11C; /* 0xFFFFFFFF 0xD4009345 0x64B5C42D 0xA1FE0F74 */
} pod_header_pod3_t;

/* POD3 entry data structure */
typedef struct pod3_entry_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod3_entry_t;

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

/* POD3 file data structure */
typedef struct pod_file_pod3_s
{
	pod_header_pod3_t* header;
	pod_byte_t* entry_data;
	pod_entry_pod3_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_audit_entry_pod3_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_size_t path_data_size;
	pod_size_t entry_data_size;

	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data;
	/* end of not serialized content */
} pod_file_pod3_t;


bool is_pod3(char* ident);

#endif
