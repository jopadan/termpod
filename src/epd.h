#ifndef _EPD_H
#define _EPD_H

#include "pod_common.h"

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
typedef struct epd_entry_s {
	pod_char_t name[POD_DIR_ENTRY_EPD_FILENAME_SIZE];
	pod_number_t size;
	pod_number_t offset;
	pod_number_t timestamp;
	pod_number_t checksum;
} epd_entry_t;

bool pod_is_epd(char* ident);


#endif
