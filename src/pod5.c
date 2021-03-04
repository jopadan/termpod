#include "pod_common.h"
#include "pod5.h"

const char pod5_audit_action_string[POD5_AUDIT_ACTION_SIZE][8] = { "Add", "Remove", "Change" };

bool pod_is_pod5(char* ident)
{
  return (POD5 == pod_type(ident) >= 0);
}

uint32_t pod_crc_pod5(pod_file_pod5_t* file)
{
	if(file == NULL || file->path_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod5() file == NULL!");
		return 0;
	}

	return crc_ccitt32_ffffffff(file->data + POD_IDENT_SIZE + POD_HEADER_CHECKSUM_SIZE, file->size - POD_IDENT_SIZE - POD_HEADER_CHECKSUM_SIZE);
}

uint32_t pod_crc_pod5_entry(pod_file_pod5_t* file, pod_number_t entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod5() file == NULL!");
		return 0;
	}

	return crc_ccitt32_ffffffff(file->data + file->entries[entry_index].offset, file->entries[entry_index].size);
}

pod_file_pod5_t* pod_file_pod5_create(pod_string_t filename)
{
	pod_file_pod5_t* pod_file = calloc(1, sizeof(pod_file_pod5_t));
	struct stat sb;
	if(stat(filename, &sb) != 0 || sb.st_size == 0)
	{
		perror("stat");
		return NULL;
	}

	pod_file->filename = calloc(1, strlen(filename));
	pod_file->filename = strcpy(pod_file->filename, filename);
	pod_file->size = sb.st_size;

	FILE* file = fopen(filename, "rb");

	if(!file)
	{
		fprintf(stderr, "ERROR: Could not open POD file: %s\n", filename);
		return NULL;
	}

	pod_file->data = calloc(1, pod_file->size);

	if(!pod_file->data)
	{
		fprintf(stderr, "ERROR: Could not allocate memory of size %zu for file %s!\n", pod_file->size, filename);
		fclose(file);
		pod_file_pod5_destroy(pod_file);
		return NULL;
	}

	if(fread(pod_file->data, POD_BYTE_SIZE, pod_file->size, file) != pod_file->size * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file %s!\n", filename);
		fclose(file);
		pod_file_pod5_destroy(pod_file);
		return NULL;
	}

	fclose(file);
	pod_file->checksum = pod_crc(pod_file->data, pod_file->size);

	size_t data_pos = 0;
	pod_file->header = (pod_header_pod5_t*)pod_file->data;
	data_pos += POD_HEADER_POD5_SIZE;
	pod_file->entries = (pod_entry_pod5_t*)(pod_file->data + data_pos);
	data_pos += pod_file->header->file_count * POD_DIR_ENTRY_POD5_SIZE;

	pod_number_t min_path_index = 0;
	pod_number_t max_path_index = 0;
	pod_number_t min_entry_index = 0;
	pod_number_t max_entry_index = 0;

	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		if(pod_file->entries[i].path_offset < pod_file->entries[min_path_index].path_offset)
		{
			min_path_index = i;
		}
		if(pod_file->entries[i].path_offset > pod_file->entries[max_path_index].path_offset)
		{
			max_path_index = i;
		}
		if(pod_file->entries[i].offset < pod_file->entries[min_entry_index].offset)
		{
			min_entry_index = i;
		}
		if(pod_file->entries[i].offset > pod_file->entries[max_entry_index].offset)
		{
			max_entry_index = i;
		}
	}


	pod_file->path_data = (pod_char_t*) (pod_file->data + data_pos);
	size_t max_path_len = strlen(pod_file->path_data + pod_file->entries[max_path_index].path_offset) + 1;

	pod_file->path_data_size = (pod_file->path_data + pod_file->entries[max_path_index].path_offset + max_path_len) - 
				(pod_file->path_data + pod_file->entries[min_entry_index].path_offset);

	size_t max_entry_len = pod_file->entries[max_entry_index].size;
	pod_file->entry_data_size = (pod_file->data + pod_file->entries[max_entry_index].offset + max_entry_len) - 
				 (pod_file->data + pod_file->entries[min_entry_index].offset);

	pod_file->entry_data = pod_file->data + pod_file->entries[min_entry_index].offset;

	data_pos += pod_file->path_data_size + pod_file->entry_data_size;

	pod_file->audit_trail = (pod_audit_entry_pod5_t*)(pod_file->data + data_pos);

	return pod_file;
}

bool pod_file_pod5_destroy(pod_file_pod5_t* podfile)
{
}

bool pod_audit_entry_pod5_print(pod_audit_entry_pod5_t* audit)
{
	if(audit == NULL)
	{
		fprintf(stderr, "ERROR: pod_audit_entry_pod5_print(audit == NULL)!\n");
		return false;
	}

	printf("\n%s %s\n%s %s\n%u / %u\n%s / %s\n",
		audit->user,
		pod_ctime(&audit->timestamp),
		pod5_audit_action_string[audit->action],
		audit->path,
		audit->old_size,
		audit->new_size,
		pod_ctime(&audit->old_timestamp),
		pod_ctime(&audit->new_timestamp));
	return true;
}

bool pod_file_pod5_print(pod_file_pod5_t* pod_file)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod5_print() podfile == NULL\n");
		return false;
	}

	/* print entries */
	printf("\nEntries:\n");
	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		pod_entry_pod5_t* entry = &pod_file->entries[i];
		pod_char_t* name = pod_file->path_data + entry->path_offset;
		printf("%10u %10u %.8X/%.8X %10u/%10u %u %s %s %10u\n",
		       	i,
			entry->offset,
			entry->checksum,
			pod_crc_pod5_entry(pod_file, i),
			entry->size,
			entry->uncompressed,
			entry->compression_level,
			pod_ctime(&entry->timestamp),
			name,
			entry->path_offset);
	}

	printf("\nAudit:\n");
	/* print audit trail */
	for(int i = 0; i < pod_file->header->audit_file_count; i++)
	{
		if(!pod_audit_entry_pod5_print(&pod_file->audit_trail[i]))
		{
			fprintf(stderr, "ERROR: pod_audit_entry_pod5_print() failed!");
			pod_file_pod5_destroy(pod_file);
			return false;
		}
	}
	/* print file summary */
	printf("\nSummary:\n \
	        file checksum      : %.8X\n \
	        size               : %zu\n \
		filename           : %s\n \
		format             : %s\n \
		comment            : %s\n \
		data checksum      : %.8X/%.8X\n \
		file entries       : %u\n \
		audit entries      : %u\n \
		revision           : %u\n \
		priority           : %u\n \
		author             : %s\n \
		copyright          : %s\n \
		index_offset       : %10u\n \
		unknown0           : %.8X\n \
		size_index         : %10u\n \
		number_min         : %.8X\n \
		number_max         : %.8X\n \
		unknown1           : %.8X\n \
		next_archive       : %s\n",
		pod_file->checksum,
		pod_file->size,
		pod_file->filename,
		pod_type_str(pod_type(pod_file->header->ident)),
		pod_file->header->comment,
		pod_file->header->checksum,
		pod_crc_pod5(pod_file),
		pod_file->header->file_count,
		pod_file->header->audit_file_count,
		pod_file->header->revision,
		pod_file->header->priority,
		pod_file->header->author,
		pod_file->header->copyright,
		pod_file->header->index_offset,
		pod_file->header->unknown10c,
		pod_file->header->size_index,
		pod_file->header->number_min,
		pod_file->header->number_max,
		pod_file->header->unknown11C,
		pod_file->header->next_archive);
	
	return true;
}

bool pod_file_pod5_write(pod_file_pod5_t* pod_file, pod_string_t filename);
bool pod_audit_entry_pod5_print(pod_audit_entry_pod5_t* audit);
bool pod_file_pod5_extract(pod_file_pod5_t* pod_file, pod_string_t dst, pod_bool_t absolute);
