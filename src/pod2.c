#include "pod_common.h" 
#include "pod2.h" 

const char pod2_audit_action_string[POD2_AUDIT_ACTION_SIZE][8] = { "Add", "Remove", "Change" };

pod_checksum_t pod_crc_pod2(pod_file_pod2_t* file)
{
	if(file == NULL || file->path_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod2() file == NULL!");
		return 0;
	}

	return pod_crc(file->data + POD_IDENT_SIZE + POD_HEADER_CHECKSUM_SIZE, file->size - POD_IDENT_SIZE - POD_HEADER_CHECKSUM_SIZE);
}

pod_checksum_t pod_crc_pod2_entry(pod_file_pod2_t* file, pod_number_t entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod2() file == NULL!");
		return 0;
	}

	return pod_crc(file->data + file->entries[entry_index].offset, file->entries[entry_index].size);
}
 
pod_checksum_t   pod_file_pod2_chksum(pod_file_pod2_t* podfile)
{
	return pod_crc_pod2(podfile);
}

bool pod_is_pod2(char* ident)
{
  return (POD2 == pod_type(ident) >= 0);
}

bool pod_audit_entry_pod2_print(pod_audit_entry_pod2_t* audit)
{
	if(audit == NULL)
	{
		fprintf(stderr, "ERROR: pod_audit_entry_pod2_print(audit == NULL)!\n");
		return false;
	}

	printf("\n%s %s\n%s %s\n%u / %u\n%s / %s\n",
		audit->user,
		pod_ctime(&audit->timestamp),
		pod2_audit_action_string[audit->action],
		audit->path,
		audit->old_size,
		audit->new_size,
		pod_ctime(&audit->old_timestamp),
		pod_ctime(&audit->new_timestamp));
	return true;
}

bool pod_file_pod2_print(pod_file_pod2_t* pod_file)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod2_print() podfile == NULL\n");
		return false;
	}

	/* print entries */
	printf("\nEntries:\n");
	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		pod_entry_pod2_t* entry = &pod_file->entries[i];
		pod_char_t* name = pod_file->path_data + entry->path_offset;
		printf("%10u %10u %.8X/%.8X %10u %s %s %10u\n",
		       	i,
			entry->offset,
			entry->checksum,
			pod_crc_pod2_entry(pod_file, i),
			entry->size,
			pod_ctime(&entry->timestamp),
			name,
			entry->path_offset);
	}

	printf("\nAudit:\n");
	/* print audit trail */
	for(int i = 0; i < pod_file->header->audit_file_count; i++)
	{
		if(!pod_audit_entry_pod2_print(&pod_file->audit_trail[i]))
		{
			fprintf(stderr, "ERROR: pod_audit_entry_pod2_print() failed!");
			pod_file = pod_file_pod2_delete(pod_file);
			return false;
		}
	}

	/* print file summary */
	printf("\nSummary:\nfile checksum      : 0x%.8X\nsize               : %zu\nfilename           : %s\nformat             : %s\ncomment            : %s\ndata checksum      : 0x%.8X/0x%.8X\nfile entries       : 0x%.8X/%.10u\naudit entries      : 0x%.8X/%.10u\n",
		pod_file->checksum,
		pod_file->size,
		pod_file->filename,
		pod_type_desc_str(pod_type(pod_file->header->ident)),
		pod_file->header->comment,
		pod_file->header->checksum,
		pod_crc_pod2(pod_file),
		pod_file->header->file_count,pod_file->header->file_count,
		pod_file->header->audit_file_count,pod_file->header->audit_file_count);

	
	return true;
}

pod_file_pod2_t* pod_file_pod2_delete(pod_file_pod2_t* podfile)
{
	if(podfile != NULL)
	{
		if(podfile->data)
		{
			free(podfile->data);
			podfile->data = NULL;
			podfile->header = NULL;
			podfile->entries = NULL;
			podfile->path_data = NULL;
			podfile->entry_data = NULL;
			podfile->audit_trail = NULL;
		}
		if(podfile->filename)
		{
			free(podfile->filename);
			podfile->filename = NULL;
		}

		free(podfile);
		podfile = NULL;
	}
	else
		fprintf(stderr, "ERROR: pod_file_pod2_delete(podfile) podfile already equals NULL!\n");

	return podfile;
}

pod_file_pod2_t* pod_file_pod2_create(pod_string_t filename)
{
	pod_file_pod2_t* pod_file = calloc(1, sizeof(pod_file_pod2_t));
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
		return pod_file_pod2_delete(pod_file);
	}

	if(fread(pod_file->data, POD_BYTE_SIZE, pod_file->size, file) != pod_file->size * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file %s!\n", filename);
		fclose(file);
		return pod_file_pod2_delete(pod_file);
	}

	fclose(file);
	pod_file->checksum = pod_crc(pod_file->data, pod_file->size);

	size_t data_pos = 0;
	pod_file->header = (pod_header_pod2_t*)pod_file->data;
	data_pos += POD_HEADER_POD2_SIZE;
	pod_file->entries = (pod_entry_pod2_t*)(pod_file->data + data_pos);
	data_pos += pod_file->header->file_count * POD_DIR_ENTRY_POD2_SIZE;

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

	pod_file->audit_trail = (pod_audit_entry_pod2_t*)(pod_file->data + data_pos);

	return pod_file;
}

bool pod_file_pod2_add_entry(pod_file_pod2_t* pod_file, pod_entry_pod2_t* entry, pod_byte_t* data)
{
	if(pod_file == NULL || entry == NULL || data == NULL)
	{
		fprintf(stderr, "ERROR: pod_file, entry or data equals NULL!\n");
		return false;
	}

	
	return true;
}

bool pod_file_pod2_write(pod_file_pod2_t* pod_file, pod_string_t filename)
{
	if(pod_file == NULL || filename == NULL)
	{
		fprintf(stderr, "ERROR: pod_file or filename equals NULL!\n");
		return false;
	}
	FILE *file = fopen(filename, "wb");

	/* write pod_header_pod2_t */
	if(fwrite(pod_type_str(POD2), POD_CHAR_SIZE, POD_IDENT_SIZE, file) != POD_IDENT_SIZE)
	{
		fprintf(stderr, "ERROR: writing POD2 ident string!\n");
		fclose(file);
		return false;
	}

	/* calculate new checksum and write it */
	pod_file->header->checksum = pod_crc_pod2(pod_file);
	if(fwrite(&pod_file->header->checksum, POD_HEADER_CHECKSUM_SIZE, 1, file) != 1)
	{
		fprintf(stderr, "ERROR: writing checksum!\n");
		fclose(file);
		return false;
	}
	/* write comment string */
	if(fwrite(pod_file->header->comment, 1, POD_COMMENT_SIZE, file) != POD_COMMENT_SIZE)
	{
		fprintf(stderr, "ERROR: writing comment string!\n");
		fclose(file);
		return false;
	}
	/* write file count */
	if(fwrite(&pod_file->header->file_count, POD_NUMBER_SIZE, 1, file) != 1)
	{
		fprintf(stderr, "ERROR: writing file count!\n");
		fclose(file);
		return false;
	}
	/* write audit file count */
	if(fwrite(&pod_file->header->audit_file_count, POD_NUMBER_SIZE, 1, file) != 1)
	{
		fprintf(stderr, "ERROR: writing audit file count!\n");
		fclose(file);
		return false;
	}
	/* write entries */
	if(fwrite(pod_file->entries, POD_DIR_ENTRY_POD2_SIZE, pod_file->header->file_count, file) != pod_file->header->file_count)
	{
		fprintf(stderr, "ERROR: writing entries!\n");
		fclose(file);
		return false;
	}
	/* write path data */
	if(fwrite(pod_file->path_data, 1, pod_file->path_data_size, file) != pod_file->path_data_size)
	{
		fprintf(stderr, "ERROR: writing path data!\n");
		fclose(file);
		return false;
	}
	/* write entry data */
	if(fwrite(pod_file->entry_data, 1, pod_file->entry_data_size, file) != pod_file->entry_data_size)
	{
		fprintf(stderr, "ERROR: writing entry data!\n");
		fclose(file);
		return false;
	}
	/* write audit trail */

	if(fwrite(pod_file->audit_trail, POD_AUDIT_ENTRY_POD2_SIZE, pod_file->header->audit_file_count, file) != pod_file->header->audit_file_count)
	{
		fprintf(stderr, "ERROR: writing audit trail!\n");
		fclose(file);
		return false;
	}

	fclose(file);
	return true;	
}

/* Extract POD2 file pod_file to directory dst                       */
/* @returns true on success otherwise false and leaves errno         */
bool pod_file_pod2_extract(pod_file_pod2_t* pod_file, pod_string_t dst, pod_bool_t absolute)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod2_extract(pod_file == NULL)\n");
		return false;
	}

	if(dst == NULL) { dst = "./"; }

	pod_char_t cwd[POD_SYSTEM_PATH_SIZE];
	getcwd(cwd, sizeof(cwd));
	printf("cwd: %s\n",cwd);
	pod_path_t root = pod_path_system_root();

	printf("root: %s\n",root);
	pod_path_t path = NULL;

	if(absolute)
	{
		path = pod_path_append_posix(root, dst);
		if(!path)
		{
			fprintf(stderr, "ERROR: pod_path_append_posix(%s,%s)\n", root, dst);
			free(root);
			return false;
		}
	}
	else
	{
		path = pod_path_append_posix(cwd, dst);
		if(!path)
		{
			fprintf(stderr, "ERROR: pod_path_append_posix(%s,%s)\n", cwd, dst);
			free(root);
			return false;
		}
	}

	printf("path: %s\n", path);
	/* create and change to destination directory */
	if(mkdir_p(path, ACCESSPERMS) != 0)
	{
		fprintf(stderr, "mkdir_p(\"%s\", \'%c\') = %s\n", dst, '/', strerror(errno));
		return false;
	}

	if(chdir(path) != 0)
	{
		fprintf(stderr, "chdir(%s) = %s\n", path, strerror(errno));
		return false;
	}

	/* extract entries */
	for(int i = 0; i < pod_file->header->file_count; i++)
	{
		pod_string_t filename = strdup(pod_file->path_data + pod_file->entries[i].path_offset);
		if(!filename)
		{
			fprintf(stderr, "ERROR: pod_file_pod2_extract(filename == NULL)\n");
			return false;
		}
		/* open and create directories including parents */
		if(mkdir_p(filename, ACCESSPERMS) != 0)
		{
			fprintf(stderr, "ERROR: mkdir_p(%s) failed: %s\n", filename, strerror(errno));
			return false;
		}
		FILE* file = fopen(filename, "wb");
		if(file == NULL)
		{
			fprintf(stderr, "ERROR: pod_fopen_mkdir(%s) failed: %s\n", filename, strerror(errno));
			return false;
		}
		if(fwrite(pod_file->data + pod_file->entries[i].offset, pod_file->entries[i].size, 1, file) != 1)
		{
			fprintf(stderr, "ERROR: fwrite failed!\n");
			fclose(file);
			return false;
		}
		/* clean up and pop */
		fclose(file);
		free(filename);
	}
	chdir(cwd);
	return true;
}
