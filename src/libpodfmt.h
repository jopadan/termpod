#ifndef _LIBPODFMT_H
#define _LIBPODFMT_H

#include <stdint.h>
#include "pod_common.h"
#include "pod1.h"
#include "pod2.h"
#include "pod3.h"
#include "pod4.h"
#include "pod5.h"
#include "pod6.h"
#include "epd.h"
#include "pod_zip.h"

typedef union pod_file_type_u
{
	pod_file_pod1_t* pod1;
	pod_file_pod2_t* pod2;
	pod_file_pod3_t* pod3;
	pod_file_pod4_t* pod4;
	pod_file_pod5_t* pod5;
	pod_file_pod6_t* pod6;
	pod_file_epd_t*  epd;
	pod_char_t       ident[__WORDSIZE];
} pod_file_type_t;

typedef union pod_entry_type_u
{
	pod_entry_pod1_t* pod1;
	pod_entry_pod2_t* pod2;
	pod_entry_pod3_t* pod3;
	pod_entry_pod4_t* pod4;
	pod_entry_pod5_t* pod5;
	pod_entry_pod6_t* pod6;
	pod_entry_epd_t*  epd;
} pod_entry_type_t;

pod_file_type_t  pod_file_create(pod_path_t name);
pod_file_type_t  pod_file_delete(pod_file_type_t file);
pod_checksum_t   pod_file_chksum(pod_file_type_t file);
int              pod_file_typeid(pod_file_type_t file);

pod_ssize_t      pod_file_write(pod_file_type_t file, pod_path_t dst_name);
pod_file_type_t  pod_file_reset(pod_file_type_t file);
pod_file_type_t  pod_file_merge(pod_file_type_t file, pod_file_type_t src);
bool             pod_file_print(pod_file_type_t file);

/* access entries by name and/or number */
pod_byte_t*      pod_file_entry_data_get(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number);
pod_file_type_t  pod_file_entry_data_add(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_byte_t* data);
pod_file_type_t  pod_file_entry_data_del(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry);
pod_checksum_t   pod_file_entry_data_chk(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry);
pod_ssize_t      pod_file_entry_data_ext(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry, pod_path_t dst);

#endif
