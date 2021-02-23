#ifndef _POD6_H
#define _POD6_H

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
typedef struct pod6_entry_s {
	pod_number_t path_offset; /* names = &(SEEK_SET + name_offset + header.names_offset] */
	pod_number_t size;
	pod_number_t offset;
	pod_number_t uncompressed;
	pod_number_t compression_level;
	pod_number_t zero;
} pod6_entry_t;

bool pod_is_pod6(char* ident);

#endif
