#ifndef _LIBPODFMT_H
#define _LIBPODFMT_H

#include "pod_common.h"
#include "pod1.h"
#include "pod2.h"
#include "pod3.h"
#include "pod4.h"
#include "pod5.h"
#include "pod6.h"
#include "epd.h"
#include "pod_zip.h"
typedef union pod_file_type_u {
	pod_file_pod1_t* pod1;
	pod_file_pod2_t* pod2;
	pod_file_pod3_t* pod3;
	pod_file_pod4_t* pod4;
	pod_file_pod5_t* pod5;
	pod_file_pod6_t* pod6;
	pod_file_epd_t*  epd;
} pod_file_type_t;

#endif
