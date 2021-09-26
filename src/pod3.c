#include "pod_common.h"
#include "pod3.h"

const char pod3_audit_action_string[POD3_AUDIT_ACTION_SIZE][8] = { "Add", "Remove", "Change" };

bool pod_is_pod3(char* ident)
{
  return (POD3 == pod_type(ident));
}

uint32_t pod_crc_pod3(pod_file_pod3_t* file)
{
	if(file == NULL || file->path_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod3() file == NULL!");
		return 0;
	}
	pod_size_t size = file->size - (file->data - file->data_start);
	fprintf(stderr, "CRC of data at %p of size %zu!\n", file->data_start, size);
	return crc_ccitt32_ffffffff(file->data_start, size);
}

uint32_t pod_crc_pod3_entry(pod_file_pod3_t* file, pod_number_t entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod3() file == NULL!");
		return 0;
	}

	pod_byte_t* start = (pod_byte_t*)file->data + file->entries[entry_index].offset;
	pod_number_t size = file->entries[entry_index].size;

	return crc_ccitt32_ffffffff(start, size);
}

uint32_t pod_crc_pod3_audit_entry(pod_file_pod3_t* file, pod_number_t audit_entry_index)
{
	if(file == NULL || file->entry_data == NULL)
	{
		fprintf(stderr, "ERROR: pod_crc_pod3() file == NULL!");
		return 0;
	}

	pod_byte_t* start = (pod_byte_t*)&file->audit_trail[audit_entry_index];
	pod_number_t size = POD_AUDIT_ENTRY_POD3_SIZE;

	return crc_ccitt32_ffffffff(start, size);
}
pod_signed_number32_t pod_entry_pod3_adjacent_diff(const void* a, const void* b)
{
	
	const pod_entry_pod3_t *x = a;
	const pod_entry_pod3_t *y = b;
	pod_number_t dx = x->offset + x->size;
	pod_number_t dy = y->offset;
	return (dx == dy ? 0 : dx < dy ? -(dy-dx) : dx - dy);
}

pod_bool_t pod_file_pod3_update_sizes(pod_file_pod3_t* pod_file)
{
	/* check arguments and allocate memory */
	size_t num_entries = pod_file->header->file_count;

	if(pod_file->gap_sizes)
		free(pod_file->gap_sizes);

	pod_file->gap_sizes = calloc(num_entries, sizeof(pod_number_t));

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
	for(pod_number_t i = 1; i < num_entries; i++)
	{
		pod_file->gap_sizes[i] = ordered_offsets[i] - (ordered_offsets[i - 1] + offset_sizes[i - 1]);
		pod_file->gap_sizes[0] += pod_file->gap_sizes[i];
		printf("gap[%d]=%d\n", i, pod_file->gap_sizes[i]);
	}


	pod_size_t expected_size = pod_file->header->index_offset - POD_HEADER_POD3_SIZE;
	pod_size_t size = pod_file->entry_data_size + pod_file->gap_sizes[0];
	/* check data start */
	pod_file->data_start = pod_file->data + pod_file->entries[min_indices[0]].offset;

	/* cleanup */
	free(offset_sizes);
	free(ordered_offsets);
	free(min_indices);
	free(offsets);


	/* compare accumulated entry sizes + gap_sizes[0] to index_offset - header */
	pod_size_t sum_size = expected_size + POD_HEADER_POD3_SIZE + pod_file->header->size_index;
	/* status output */
	fprintf(stderr, "data_start: %p/%p\n", pod_file->data_start, pod_file->data + POD_HEADER_POD3_SIZE);
	fprintf(stderr, "size_diff: %zu/%u\n", size - expected_size, pod_file->gap_sizes[0]);
	fprintf(stderr, "index_offset + size_index: %u + %u = %zu", pod_file->header->index_offset, pod_file->header->size_index,
	sum_size);

	return size == expected_size;
}

/*
pod_file_pod3_t* pod_file_pod3_create(pod_string_t filename)
{
	pod_file_pod3_t* pod_file = calloc(1, sizeof(pod_file_pod3_t));
	struct stat sb;
	if(stat(filename, &sb) != 0 || sb.st_size == 0)
	{
		perror("stat");
		return NULL;
	}

	pod_file->filename = strdup(filename);
	pod_file->size = sb.st_size;
	size_t read = 0;
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
		pod_file_pod3_destroy(pod_file);
		return NULL;
	}

	if( (read = fread(pod_file->data, POD_BYTE_SIZE, pod_file->size, file)) != pod_file->size)
	{
		fprintf(stderr, "ERROR: Stopped reading file %s at %zu of %zu bytes!\n", filename, read, pod_file->size);
		fclose(file);
		pod_file_pod3_destroy(pod_file);
		return NULL;
	}

	pod_file->header = (pod_header_pod3_t*)pod_file->data;

	if(!pod_is_pod3(pod_file->header->ident))
	{
		fprintf(stderr, "ERROR: File %s is missing POD3 file magic!\n", filename);
		fclose(file);
		pod_file_pod3_destroy(pod_file);
		return NULL;
	}

	fclose(file);

	pod_file->checksum = pod_crc(pod_file->data, pod_file->size);

	pod_number_t num_entries = pod_file->header->file_count;
	pod_file->entries = (pod_entry_pod3_t*)(pod_file->data +  pod_file->header->index_offset);

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

	pod_file->entry_data = (pod_byte_t*)pod_file->data + pod_file->entries[min_entry_index].offset;
	pod_file->path_data = (pod_char_t*)((pod_byte_t*)pod_file->entries + (POD_DIR_ENTRY_POD3_SIZE * pod_file->header->file_count));

	for(int i = 0; i < pod_file->header->file_count; i++)
		pod_file->path_data + pod_file->entries[i].path_offset;

	size_t max_path_len = strlen((pod_char_t*)(pod_file->entries + pod_file->entries[max_path_index].path_offset)) + 1;
	size_t max_entry_len = pod_file->entries[max_entry_index].size;


	if(pod_file->header->audit_file_count > 0)
	{
		pod_file->audit_trail = (pod_audit_entry_pod3_t*)((pod_byte_t*)pod_file->path_data + pod_file->header->size_index);
		pod_file->audit_data_size = pod_file->header->audit_file_count * POD_AUDIT_ENTRY_POD3_SIZE;
	}

	if(!pod_file_pod3_update_sizes(pod_file))
	{
		fprintf(stderr, "ERROR: Could not update POD3 file entry sizes\n");
		pod_file_pod3_destroy(pod_file);
		return NULL;
	}

	return pod_file;
}
*/

pod_file_pod3_t* pod_file_pod3_create(pod_string_t filename, uint8_t size_index) {
    pod_number_t ecx4;
    pod_number_t edx5;
    pod_number_t edx6;
    pod_number_t esi7;
    pod_number_t v8;
    pod_number_t v9;

    /* read POD data */
    struct stat sb;
    size_t bytes = 0;
    size_t read = 0;

    char *rotorchar = "-/|\\";
    uint8_t rotor = 0;

    if(stat(filename, &sb) != 0)
	    return NULL;

    FILE* file = fopen(filename, "rb");
    if(!file)
	    return NULL;

    /* allocate memory */
    pod_file_pod3_t* pod_file = calloc(1, sizeof(pod_file_pod3_t));
    if(!pod_file)
    {
    	fclose(file);
    	return NULL;
    }

    pod_file->data = calloc(sb.st_size,sizeof(uint8_t));
    if(!pod_file->data)
    {
    	fclose(file);
	free(pod_file);
    	return NULL;
    }

    pod_file->header = (pod_header_pod3_t*)pod_file->data;
    pod_file->filename = strdup(filename);
    pod_file->size = sb.st_size;

    /* read data as sizeof(quad_t) steps */
    quad_t* ptr = (quad_t*)&pod_file->data[0];

    for(read = 0; read < sb.st_size; read += fread(ptr++, sizeof(uint8_t), sizeof(quad_t), file))
    {
	    fprintf(stdout, "\rLoading POD file... %lu/%lu (%c)", read, sb.st_size, rotorchar[rotor]); 
	    fflush(stdout);
	    rotor++;
	    rotor&=3;
    }

    if( read != sb.st_size)
    {
	    fprintf(stdout, "\rLoading POD file... %lu/%lu FAILED!\n", read, sb.st_size);
	    fflush(stdout);
	    fclose(file);
	    pod_file_pod3_destroy(pod_file);
	    return NULL;
    }

    fprintf(stdout, "\rLoading POD file... %lu/%lu SUCCESS!\n", read, sb.st_size, rotorchar[rotor]);

    /* read header */
    uint8_t iudiff = *(uint8_t*)&pod_file->header->index_offset - *(uint8_t*)&pod_file->header->size_index;
    uint8_t isdiff = *(int8_t*)&pod_file->header->index_offset - *(int8_t*)&pod_file->header->size_index;
    size_t size = 0;

    if(!pod_is_pod3(pod_file->header->ident))
    {
	    fprintf(stdout, "\rReading POD file magic... %u/%u FAILED!\n", 0,4);
	    fflush(stdout);
	    fclose(file);
	    pod_file_pod3_destroy(pod_file);
	    return NULL;
    }
    else
    	fprintf(stdout, "\rReading POD file magic... %u/%u SUCCESS!\n", 4,4);

    if((pod_file->header->checksum = *(pod_number_t*)(pod_file->data + 4)) == 0)
	    pod_file->header->checksum = 0xfffffffe;
    else if(*(int8_t*)&pod_file->header->index_offset <= *(int8_t*)&pod_file->header->size_index)
    {
	    printf("Loading index_offset: %02x/%08x size_index: %02x/%08x\n", *(int8_t*)&pod_file->header->index_offset, pod_file->header->index_offset, *(int8_t*)&pod_file->header->size_index, pod_file->header->size_index);
	    pod_file->header->checksum = 0xffffffff;
    }

    if( *(int8_t*)&size > *(int8_t*)&iudiff)
	    size = iudiff;

    pod_file->data_offset = pod_file->header->pad10c + size_index;
    (*(uint16_t*)&pod_file->data_offset) &= 0xf000;
    ecx4 = pod_file->data_offset - *(uint8_t*)&pod_file->header->pad10c;
    edx5 = *(uint8_t*)&pod_file->header->size_index - *(uint8_t*)&ecx4;
    pod_file->header->pad124 = edx5;
    edx6 = edx5 + 0xfff;
    pod_file->header->pad11c = ecx4;
    (*(uint16_t*)&edx6) &= 0xf000;
    esi7 = pod_file->header->pad120;
    pod_file->header->pad124 = edx6;
    if (*(int8_t*)&edx6 > *(int8_t*)&pod_file->header->pad120)
        pod_file->header->pad124 = esi7;

    pod_file->header->pad124 = pod_file->header->checksum;
    /*
    SetFilePointer(header->checksum, data_start, 0, 0);
    v9 = header->checksum;
    ReadFile();
    header->pad124 = v9;
*/
    if (*(int8_t*)&v9 < 1)
        pod_file->header->pad124 = 0;

    fprintf(stdout, "\rReading POD file checksum... %08x SUCCESS!\n", pod_file->checksum);
    return pod_file;
}


bool pod_file_pod3_destroy(pod_file_pod3_t* podfile)
{
	if(!podfile)
	{
		fprintf(stderr, "ERROR: could not free podfile == NULL!\n");
		return false;
	}

	if(podfile->gap_sizes)
		free(podfile->gap_sizes);
	if(podfile->data)
		free(podfile->data);
	if(podfile->filename);
		free(podfile->filename);
	if(podfile)
		free(podfile);
	return true;
}

bool pod_audit_entry_pod3_print(pod_audit_entry_pod3_t* audit)
{
	if(audit == NULL)
	{
		fprintf(stderr, "ERROR: pod_audit_entry_pod3_print(audit == NULL)!\n");
		return false;
	}
	printf("\n%s %s\n%s %s\n%u / %u\n%s / %s\n",
		audit->user,
		pod_ctime(&audit->timestamp),
		pod3_audit_action_string[audit->action],
		audit->path,
		audit->old_size,
		audit->new_size,
		pod_ctime(&audit->old_timestamp),
		pod_ctime(&audit->new_timestamp));

	return true;
}

bool pod_file_pod3_print(pod_file_pod3_t* pod_file)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "ERROR: pod_file_pod3_print() podfile == NULL\n");
		return false;
	}

	/* print entries */
	printf("\nEntries:\n");
	for(pod_number_t i = 0; i < pod_file->header->file_count; i++)
	{
		pod_entry_pod3_t* entry = &pod_file->entries[i];
		pod_char_t* name = pod_file->path_data + entry->path_offset;
		printf("%.10u %.10u %.8X/%.8X %10u %.32s %.32s %10u\n",
		       	i,
			entry->offset,
			entry->checksum,
			pod_crc_pod3_entry(pod_file, i),
			entry->size,
			pod_ctime(&entry->timestamp),
			name,
			entry->path_offset);
	}
/*
	if(pod_file->audit_trail != NULL || pod_file->audit_data_size > 0)
	{
		printf("\nAudit:\n");
		for(int i = 0; i < pod_file->header->audit_file_count; i++)
		{
			if(!pod_audit_entry_pod3_print(&pod_file->audit_trail[i]))
			{
				fprintf(stderr, "ERROR: pod_audit_entry_pod3_print() failed!");
				pod_file_pod3_destroy(pod_file);
				return false;
			}
		}
	}
*/
	/* print file summary */
	printf("\nSummary:\n \
	        file checksum      : 0x%.8X\n \
	        size               : %.10zu\n \
		filename           : %s\n \
		format             : %s\n \
		comment            : %s\n \
		data checksum      : 0x%.8X/0x%.8X\n \
		file entries       : 0x%.8X/%.10u\n \
		audit entries      : 0x%.8X/%.10u\n \
		revision           : 0x%.8X/%.10u\n \
		priority           : 0x%.8X/%.10u\n \
		author             : %s\n \
		copyright          : %s\n \
		index_offset       : 0x%.8X/%.10u\n \
		pad272_10C         : 0x%.8X/0x%.8X\n \
		size_index         : 0x%.8X/%.10u\n \
		number_min         : 0x%.8X/%.10u\n \
		number_max         : 0x%.8X/%.10u\n \
		pad_11c            : 0x%.8X/0x%.8X\n \
		pad_120            : 0x%.8X/0x%.8X\n \
		pad_124            : 0x%.8X/0x%.8X\n",
		pod_file->checksum,
		pod_file->size,
		pod_file->filename,
		pod_type_str(pod_type(pod_file->header->ident)),
		pod_file->header->comment,
		pod_file->header->checksum,
		pod_crc_pod3(pod_file),
		pod_file->header->file_count,pod_file->header->file_count,
		pod_file->header->audit_file_count,pod_file->header->audit_file_count,
		pod_file->header->revision,pod_file->header->revision,
		pod_file->header->priority,pod_file->header->priority,
		pod_file->header->author,
		pod_file->header->copyright,
		pod_file->header->index_offset,pod_file->header->index_offset,
		pod_file->header->pad10c,
		pod_crc_pod3(pod_file),
		pod_file->header->size_index,pod_file->header->size_index,
		pod_file->header->number_min,pod_file->header->number_min,
		pod_file->header->number_max,pod_file->header->number_max,
		pod_file->header->pad11c,pod_file->header->pad11c,
		pod_file->header->pad120,pod_file->header->pad120,
		pod_file->header->pad124,pod_file->header->pad124);
	
	return true;
}

bool pod_file_pod3_write(pod_file_pod3_t* pod_file, pod_string_t filename);
