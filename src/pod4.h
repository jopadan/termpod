#ifndef _POD4_H
#define _POD4_H

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
	pod_number_t unknown10c;
	pod_number_t size_index;
	pod_number_t number_min; /* 0x00000000 */
	pod_number_t number_max; /* 0xFFFFFFFF */
	pod_number_t unknown11C;
} pod_header_pod4_t;

/* POD4 entry data structure */
typedef struct pod4_entry_s {
	pod_number_t path_offset;
	pod_number_t size;
	pod_number_t offset;
	pod_number_t uncompressed;
	pod_number_t compression_level;
	pod_number_t timestamp;
	pod_number_t checksum;
} pod4_entry_t;

/* POD4 file data structure */
typedef struct pod_file_pod4_s
{
	pod_header_pod4_t* header;
	pod_byte_t* entry_data;
	pod_entry_pod4_t* entries; /* header.file_count */
	pod_char_t* path_data;
	pod_audit_entry_pod4_t* audit_trail; /* header.audit_file_count */
	/* not serialized content */
	pod_size_t path_data_size;
	pod_size_t entry_data_size;
	pod_size_t audit_data_size;
	pod_string_t filename;
	pod_size_t size;
	pod_number_t checksum;
	pod_byte_t* data;
	pod_zip_entry_pod4_t* zip_entries;
	/* end of not serialized content */
} pod_file_pod4_t;


bool is_pod4(char* ident);

#endif
