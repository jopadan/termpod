#include "libpodfmt.h"

int              pod_file_typeid(pod_file_type_t file) { return file.pod1 != NULL ? pod_type(file.ident) : -1; }

pod_file_type_t pod_file_create(pod_path_t name)
{
	pod_file_type_t pod = { NULL };
	if(name == NULL)
	{
		fprintf(stderr, "ERROR: path == NULL!");
		return pod;
	}


	switch(pod_type_peek(name))
	{
		case POD1:
			pod.pod1 = pod_file_pod1_create(name);
			if(pod.pod1 == NULL)
				fprintf(stderr, "ERROR: cannot create pod1 file!\n");
			break;
		case POD2:
			pod.pod2 = pod_file_pod2_create(name);
			if(pod.pod2 == NULL)
				fprintf(stderr, "ERROR: cannot create pod2 file!\n");
			break;
		case POD3:
			pod.pod3 = pod_file_pod3_create(name);
			if(pod.pod3 == NULL)
				fprintf(stderr, "ERROR: cannot create pod3 file!\n");
			break;
		case POD4:
			pod.pod4 = pod_file_pod4_create(name);
			if(pod.pod4 == NULL)
				fprintf(stderr, "ERROR: cannot create pod4 file!\n");
			break;
		case POD5:
			pod.pod5 = pod_file_pod5_create(name);
			if(pod.pod5 == NULL)
				fprintf(stderr, "ERROR: cannot create pod5 file!\n");
			break;
		case POD6:
			pod.pod6 = pod_file_pod6_create(name);
			if(pod.pod6 == NULL)
				fprintf(stderr, "ERROR: cannot create pod6 file!\n");
			break;
		case EPD:
			pod.epd = pod_file_epd_create(name);
			if(pod.epd == NULL)
				fprintf(stderr, "ERROR: cannot create epd file!\n");
			break;
		default:
			fprintf(stderr, "ERROR: unknown file format!\n");
			break;
	}
	return pod;
}

pod_file_type_t pod_file_delete(pod_file_type_t file)
{
	if(file.pod1 != NULL)
	{
		switch(pod_file_typeid(file))
		{
			case POD1:
				file.pod1 = pod_file_pod1_delete(file.pod1);
				break;
			case POD2:
				file.pod2 = pod_file_pod2_delete(file.pod2);
				break;
			case POD3:
				file.pod3 = pod_file_pod3_delete(file.pod3);
				break;
			case POD4:
				file.pod4 = pod_file_pod4_delete(file.pod4);
				break;
			case POD5:
				file.pod5 = pod_file_pod5_delete(file.pod5);
				break;
			case POD6:
				file.pod6 = pod_file_pod6_delete(file.pod6);
				break;
			case EPD:
				file.epd = pod_file_epd_delete(file.epd);
				break;
			default:
				fprintf(stderr, "ERROR: pod_file_delete() unknown file format!\n");
				free(file.pod1);
				file.pod1 = NULL;
				break;
		}
	}
	return file;
}

pod_checksum_t   pod_file_chksum(pod_file_type_t file)
{
	pod_checksum_t checksum = 0xFFFFFFFF;
	if(file.pod1 != NULL)
	{
		switch(pod_file_typeid(file))
		{
			case POD1:
				checksum = pod_file_pod1_chksum(file.pod1);
				break;
			case POD2:
				checksum = pod_file_pod2_chksum(file.pod2);
				break;
			case POD3:
				checksum = pod_file_pod3_chksum(file.pod3);
				break;
			case POD4:
				checksum = pod_file_pod4_chksum(file.pod4);
				break;
			case POD5:
				checksum = pod_file_pod5_chksum(file.pod5);
				break;
			case POD6:
				checksum = pod_file_pod6_chksum(file.pod6);
				break;
			case EPD:
				checksum = pod_file_epd_chksum(file.epd);
				break;
			default:
				fprintf(stderr, "ERROR: pod_file_chksum() unknown file format!\n");
				break;
		}
	}
	return checksum;
}

pod_ssize_t      pod_file_write(pod_file_type_t file, pod_path_t dst_name);
pod_file_type_t  pod_file_reset(pod_file_type_t file);
pod_file_type_t  pod_file_merge(pod_file_type_t file, pod_file_type_t src);

/* access entries by name and/or number */
pod_byte_t*      pod_file_entry_data_get(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number);
pod_file_type_t  pod_file_entry_data_add(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_byte_t* data);
pod_file_type_t  pod_file_entry_data_del(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry);
pod_checksum_t   pod_file_entry_data_chk(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry);
pod_ssize_t      pod_file_entry_data_ext(pod_file_type_t file, pod_path_t entry_name, pod_number_t entry_number, pod_entry_type_t* entry, pod_path_t dst);


