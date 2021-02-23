#ifndef _POD1_H
#define _POD1_H

/* POD1 header data structure */
typedef struct pod_header_pod1_s
{
	pod_number_t file_count;
	pod_char_t comment[POD_HEADER_COMMENT_SIZE];
} pod_header_pod1_t;

/* POD1 entry data structure */
typedef struct pod1_entry_s {
	pod_char_t name[POD_DIR_ENTRY_POD1_FILENAME_SIZE];
	pod_number_t size;
	pod_number_t offset;
} pod1_entry_t;

bool pod_is_pod1(char* ident);

#endif
