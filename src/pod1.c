#include "pod_common.h"
#include "pod1.h"

bool pod_is_pod1(char* ident)
{
  return (POD1 == pod_type(ident) >= 0);
}

uint32_t pod_crc_pod1(pod_file_pod1_t* file)
{
	if(file == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod1() file == NULL!");
		return 0;
	}

	return crc_ccitt32_ffffffff(file->data + POD_IDENT_SIZE + POD_HEADER_CHECKSUM_SIZE, file->size - POD_IDENT_SIZE - POD_HEADER_CHECKSUM_SIZE);
}

uint32_t pod_crc_pod1_entry(pod_file_pod1_t* file, pod_number_t entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod1() file == NULL!");
		return 0;
	}

	return crc_ccitt32_ffffffff(file->data + file->entries[entry_index].offset, file->entries[entry_index].size);
}
 

pod_file_pod1_t* pod_file_pod1_create(pod_string_t filename)
{
	pod_file_pod1_t* pod_file = calloc(1, sizeof(pod_file_pod1_t));
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
		pod_file_pod1_destroy(pod_file);
		return NULL;
	}

	if(fread(pod_file->data, POD_BYTE_SIZE, pod_file->size, file) != pod_file->size * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file %s!\n", filename);
		fclose(file);
		pod_file_pod1_destroy(pod_file);
		return NULL;
	}

	fclose(file);
	pod_file->checksum = pod_crc(pod_file->data, pod_file->size);

	size_t data_pos = 0;
	pod_file->header = (pod_header_pod1_t*)pod_file->data;
	data_pos += POD_HEADER_POD1_SIZE;
	pod_file->entries = (pod_entry_pod1_t*)(pod_file->data + data_pos);
	data_pos += pod_file->header->file_count * POD_DIR_ENTRY_POD1_SIZE;

	pod_number_t min_entry_index = 0;
	pod_number_t max_entry_index = 0;

	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		if(pod_file->entries[i].offset < pod_file->entries[min_entry_index].offset)
		{
			min_entry_index = i;
		}
		if(pod_file->entries[i].offset > pod_file->entries[max_entry_index].offset)
		{
			max_entry_index = i;
		}
	}

	size_t max_entry_len = pod_file->entries[max_entry_index].size;
	pod_file->entry_data_size = (pod_file->data + pod_file->entries[max_entry_index].offset + max_entry_len) - 
				 (pod_file->data + pod_file->entries[min_entry_index].offset);

	pod_file->entry_data = pod_file->data + pod_file->entries[min_entry_index].offset;

	data_pos += pod_file->entry_data_size;

	return pod_file;
}

bool pod_file_pod1_add_entry(pod_file_pod1_t* pod_file, pod_entry_pod1_t* entry, pod_byte_t* data)
{
	if(pod_file == NULL || entry == NULL || data == NULL)
	{
		fprintf(stderr, "ERROR: pod_file, entry or data equals NULL!\n");
		return false;
	}

	
	return true;
}

bool pod_file_pod1_write(pod_file_pod1_t* pod_file, pod_string_t filename)
{
	if(pod_file == NULL || filename == NULL)
	{
		fprintf(stderr, "ERROR: pod_file or filename equals NULL!\n");
		return false;
	}
	FILE *file = fopen(filename, "wb");

	/* write file count */
	if(fwrite(&pod_file->header->file_count, POD_NUMBER_SIZE, 1, file) != 1)
	{
		fprintf(stderr, "ERROR: writing file count!\n");
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
	/* write entries */
	if(fwrite(pod_file->entries, POD_DIR_ENTRY_POD1_SIZE, pod_file->header->file_count, file) != pod_file->header->file_count)
	{
		fprintf(stderr, "ERROR: writing entries!\n");
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

	fclose(file);
	return true;	
}

/* Extract POD1 file pod_file to directory dst                       */
/* @returns true on success otherwise false and leaves errno         */
bool pod_file_pod1_extract(pod_file_pod1_t* pod_file, pod_string_t dst, pod_bool_t absolute)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod1_extract(pod_file == NULL)\n");
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
		/* open and create directories including parents */
		if(mkdir_p(pod_file->entries[i].name, ACCESSPERMS) != 0)
		{
			fprintf(stderr, "ERROR: mkdir_p(%s) failed: %s\n", pod_file->entries[i].name, strerror(errno));
			return false;
		}
		FILE* file = fopen(pod_file->entries[i].name, "wb");
		if(file == NULL)
		{
			fprintf(stderr, "ERROR: pod_fopen_mkdir(%s) failed: %s\n", pod_file->entries[i].name, strerror(errno));
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
	}
	chdir(cwd);
	return true;
}
bool pod_file_pod1_destroy(pod_file_pod1_t* pod_file)
{
	if(!pod_file)
	{
		fprintf(stderr, "ERROR: could not free pod_file == NULL!\n");
		return false;
	}

	if(pod_file->data)
		free(pod_file->data);
	if(pod_file->filename);
		free(pod_file->filename);
	if(pod_file)
		free(pod_file);
	return true;
}

bool pod_file_pod1_print(pod_file_pod1_t* pod_file)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod1_print() pod_file == NULL\n");
		return false;
	}

	/* print entries */
	printf("\nEntries:\n");
	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		pod_entry_pod1_t* entry = &pod_file->entries[i];
		printf("%10u 0x%.8X/%.10u 0x%.8X/%10u %s\n",
		       	i,
			entry->offset,entry->offset,
			entry->size,entry->size,
			entry->name);
	}

	/* print file summary */
	printf("\nSummary:\nfile checksum      : 0x%.8X\nsize               : %zu\nfilename           : %s\nformat             : %s\ncomment            : file entries       : 0x%.8X/%.10u\n",
		pod_file->checksum,
		pod_file->size,
		pod_file->filename,
		pod_file->header->comment,
		pod_file->header->file_count,pod_file->header->file_count);

	
	return true;
}
