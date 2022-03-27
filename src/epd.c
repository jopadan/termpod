#include "pod_common.h"
#include "epd.h"

bool pod_is_epd(char* ident)
{
	pod_ident_type_t type = pod_type(ident);
	return (type >= 0 && type == EPD);
}

uint32_t pod_crc_epd(pod_file_epd_t* file)
{
	if(file == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_epd() file == NULL!");
		return 0;
	}
	pod_byte_t* start = (pod_byte_t*)&file->data + POD_HEADER_EPD_SIZE;
	pod_size_t size = file->size - POD_HEADER_EPD_SIZE;
	fprintf(stderr, "CRC of data at %p of size %lu!\n", start, size);
	return pod_crc(start, size);
}

uint32_t pod_crc_epd_entry(pod_file_epd_t* file, pod_number_t entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_epd() file == NULL!");
		return 0;
	}

	pod_byte_t* start = (pod_byte_t*)file->data + file->entries[entry_index].offset;
	pod_number_t size = file->entries[entry_index].size;
	return pod_crc(start, size);
}

pod_signed_number32_t pod_entry_epd_adjacent_diff(const void* a, const void* b)
{
	
	const pod_entry_epd_t *x = a;
	const pod_entry_epd_t *y = b;
	pod_number_t dx = x->offset + x->size;
	pod_number_t dy = y->offset;
	return (dx == dy ? 0 : dx < dy ? -(dy-dx) : dx - dy);
}

pod_bool_t pod_file_epd_update_sizes(pod_file_epd_t* pod_file)
{
	/* check arguments and allocate memory */
	size_t num_entries = pod_file->header->file_count;

	if(pod_file->gap_sizes)
		free(pod_file->gap_sizes);

	pod_file->gap_sizes = calloc(num_entries + 1, sizeof(pod_number_t));

	if(pod_file->gap_sizes == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for gap_sizes\n");
		return false;
	}

	pod_number_t* offsets = calloc(num_entries, sizeof(pod_number_t));
	pod_number_t* min_indices = calloc(num_entries, sizeof(pod_number_t));
	pod_number_t* ordered_offsets = calloc(num_entries, sizeof(pod_number_t));
	pod_number_t* offset_sizes = calloc(num_entries, sizeof(pod_number_t));

	/* sort offset indices and accumulate entry size */
	for(pod_number_t i = 0; i < num_entries; i++)
		offsets[i] = pod_file->entries[i].offset;

	for(pod_number_t i = 0; i < num_entries; i++)
	{
		for(pod_number_t j = i; j < num_entries; j++)
		{
			if(offsets[j] <= offsets[i])
				min_indices[i] = j;
		}
	}

	pod_file->entry_data_size = 0;
	for(pod_number_t i = 0; i < num_entries; i++)
	{
		ordered_offsets[i] = offsets[min_indices[i]];
		offset_sizes[i] = pod_file->entries[min_indices[i]].size;
		pod_file->entry_data_size += offset_sizes[i];
	}

	/* find gap sizes */ 
	for(pod_number_t i = 0; i < num_entries; i++)
	{
		pod_file->gap_sizes[i] = ((i == num_entries - 1) ? pod_file->size : ordered_offsets[i + 1]) - (ordered_offsets[i] + offset_sizes[i]);
		pod_file->gap_sizes[num_entries] += pod_file->gap_sizes[i];

		fprintf(stderr, "gap[%u]=%u accum:%u\n", i, pod_file->gap_sizes[i], pod_file->gap_sizes[num_entries]);
	}

	/* check data start */
	pod_file->data_start = pod_file->data + offsets[min_indices[0]];

	/* cleanup */
	free(offset_sizes);
	free(ordered_offsets);
	free(min_indices);
	free(offsets);


	/* compare accumulated entry sizes + gap_sizes[0] to index_offset - header */
	pod_number_t size = pod_file->entry_data_size + pod_file->gap_sizes[num_entries];
	pod_number_t expected_size = pod_file->size - POD_HEADER_EPD_SIZE - POD_DIR_ENTRY_EPD_SIZE * num_entries; 
	pod_number_t sum_size = size + POD_HEADER_EPD_SIZE;
	/* status output */
	fprintf(stderr, "data_start: %lu/%lu\n", pod_file->entry_data - pod_file->data, pod_file->data_start - pod_file->data);
	fprintf(stderr, "accumulated_size: %u/%u\naccumulated gap size: %u\n", size, expected_size, pod_file->gap_sizes[num_entries]);
	fprintf(stderr, "data_start + size: %u + %u = %u", (pod_number_t)POD_HEADER_EPD_SIZE, size, sum_size);

	return size == expected_size;
}

pod_checksum_t  pod_file_epd_chksum(pod_file_epd_t* podfile)
{
	return pod_crc_epd(podfile);
}
pod_file_epd_t* pod_file_epd_create(pod_string_t filename)
{
	pod_file_epd_t* pod_file = calloc(1, sizeof(pod_file_epd_t));
	struct stat sb;
	if(stat(filename, &sb) != 0 || sb.st_size == 0)
	{
		perror("stat");
		return NULL;
	}

	pod_file->filename = strdup(filename);
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
		return pod_file_epd_delete(pod_file);
	}

	if(fread(pod_file->data, POD_BYTE_SIZE, POD_IDENT_SIZE, file) != POD_IDENT_SIZE * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file magic %s!\n", filename);
		fclose(file);
		return pod_file_epd_delete(pod_file);
	}
	if(!pod_is_epd(pod_file->data))
	{
		fprintf(stderr, "ERROR: POD file format is not EPD %s!\n", filename);
		fclose(file);
		return pod_file_epd_delete(pod_file);
	}

	if(fread(pod_file->data + POD_IDENT_SIZE, POD_BYTE_SIZE, EPD_COMMENT_SIZE, file) != EPD_COMMENT_SIZE * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file comment %s!\n", filename);
		fclose(file);
		return pod_file_epd_delete(pod_file);
	}
	if(fread(pod_file->data + POD_IDENT_SIZE + EPD_COMMENT_SIZE , POD_NUMBER_SIZE, 3, file) != 3)
	{
		fprintf(stderr, "ERROR: Could not read file header %s!\n", filename);
		fclose(file);
		return pod_file_epd_delete(pod_file);
	}
	if(fread(pod_file->data + POD_HEADER_EPD_SIZE, POD_BYTE_SIZE, pod_file->size - POD_HEADER_EPD_SIZE, file) != (pod_file->size - POD_HEADER_EPD_SIZE ) * POD_BYTE_SIZE)
	{
		fprintf(stderr, "ERROR: Could not read file %s!\n", filename);
		fclose(file);
		return pod_file_epd_delete(pod_file);
	}

	fclose(file);
	pod_file->checksum = pod_crc(pod_file->data, pod_file->size);

	pod_file->header = (pod_header_epd_t*)pod_file->data;
	pod_file->entries = (pod_entry_epd_t*)(pod_file->data + POD_HEADER_EPD_SIZE);
	pod_number_t num_entries = pod_file->header->file_count;

	pod_number_t min_entry_index = 0;
	pod_number_t max_entry_index = 0;

	for(pod_number_t i = 0; i < num_entries; i++)
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
	pod_file->entry_data = (pod_byte_t*)(pod_file->data + pod_file->entries[min_entry_index].offset);

	size_t max_entry_len = pod_file->entries[max_entry_index].size;

	if(!pod_file_epd_update_sizes(pod_file))
	{
		fprintf(stderr, "ERROR: Could not update EPD file entry sizes\n");
		pod_file = pod_file_epd_delete(pod_file);
	}

	return pod_file;
}

pod_file_epd_t* pod_file_epd_delete(pod_file_epd_t* podfile)
{
	if(podfile)
	{
		if(podfile->gap_sizes)
		{
			free(podfile->gap_sizes);
			podfile->gap_sizes = NULL;
		}
		if(podfile->data)
		{
			free(podfile->data);
			podfile->data = NULL;
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
		fprintf(stderr, "ERROR: could not free podfile == NULL!\n");

	return podfile;
}


bool pod_file_epd_print(pod_file_epd_t* pod_file)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_epd_print() podfile == NULL\n");
		return false;
	}

	/* print entries */
	printf("\nEntries:\n");
	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		pod_entry_epd_t* entry = &pod_file->entries[i];
		pod_char_t* name = pod_file->entries[i].name;
		printf("%10u %10u %10u 0x%.8X/0x%.8X %s %s\n",
		       	i,
			entry->offset,
			entry->size,
			entry->checksum, pod_crc_epd_entry(pod_file, i),
			pod_ctime(&entry->timestamp),
			name);
	}

	/* print file summary */
	printf("\nSummary:\n \
	        file checksum      : 0x%.8X/%.10u\n \
	        size               : %zu\n \
		filename           : %s\n \
		format             : %s\n \
		comment            : %s\n \
		data checksum      : 0x%.8X/0x%.8X\n \
		file entries       : 0x%.8X/%.10u\n \
		version            : 0x%.8X/%.10u\n",
		pod_file->checksum,pod_file->checksum,
		pod_file->size,
		pod_file->filename,
		pod_type_desc_str(pod_type(pod_file->header->ident)),
		pod_file->header->comment,
	        pod_file->header->checksum, pod_file->header->checksum,
		pod_file->header->file_count,pod_file->header->file_count,
		pod_file->header->version,pod_file->header->version);
	
	return true;
}

bool pod_file_epd_write(pod_file_epd_t* pod_file, pod_string_t filename);
