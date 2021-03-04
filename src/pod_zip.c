#include "pod_zip.h"

pod_number_t pod_zip_crc_ccitt32(pod_byte_t* src, pod_size_t compressed)
{
}

pod_number_t pod_zip_crc(pod_byte_t* src, pod_size_t compressed)
{
	pod_number_t crc = 0;

	if(src == NULL)
	{
		fprintf(stderr, "src is a NULL pointer argument!");
		return crc;
	}

	zip_error_t zip_error;

	/* create source from buffer */
	zip_source_t *zip_src = zip_source_buffer_create(src, compressed, 1, &zip_error);

	if(zip_src == NULL)
	{
		fprintf(stderr, "pod_zip: error creating source!\n");
		zip_error_fini(&zip_error);
		return crc;
	}
	/* open zip archive from source */
	zip_t *za = zip_open_from_source(zip_src, 0, &zip_error);
	if(za == NULL)
	{
		fprintf(stderr, "pod_zip: error opening zip from source!\n");
		zip_error_fini(&zip_error);
		return crc;
	}

	zip_error_fini(&zip_error);

	/* keep the source */
	zip_source_keep(zip_src);

	/* close archive */
	if(zip_close(za) < 0)
	{
		fprintf(stderr, "pod_zip: error closing zip archive of dst buffer: %s\n", zip_strerror(za));
		return crc;
	}

	/* stat zip size */
	if(!zip_source_is_deleted(zip_src))
	{
		zip_stat_t zst;

		if(zip_source_stat(zip_src, &zst) < 0)
		{
			fprintf(stderr, "can't stat source: %s\n", zip_error_strerror(zip_source_error(zip_src)));
			return crc;
		}
		crc = zst.crc;

		zip_source_close(zip_src);
	}

	zip_source_free(zip_src);

	return crc;
}

pod_size_t pod_zip_size(pod_byte_t* src, pod_size_t compressed)
{
	zip_uint64_t uncompressed = 0;

	if(src == NULL)
	{
		fprintf(stderr, "src is a NULL pointer argument!");
		return -1;
	}

	zip_error_t zip_error;

	/* create source from buffer */
	zip_source_t *zip_src = zip_source_buffer_create(src, compressed, 1, &zip_error);

	if(zip_src == NULL)
	{
		fprintf(stderr, "pod_zip: error creating source!\n");
		zip_error_fini(&zip_error);
		return -1;
	}
	/* open zip archive from source */
	zip_t *za = zip_open_from_source(zip_src, 0, &zip_error);
	if(za == NULL)
	{
		fprintf(stderr, "pod_zip: error opening zip from source!\n");
		zip_error_fini(&zip_error);
		return -1;
	}

	zip_error_fini(&zip_error);

	/* keep the source */
	zip_source_keep(zip_src);

	/* close archive */
	if(zip_close(za) < 0)
	{
		fprintf(stderr, "pod_zip: error closing zip archive of dst buffer: %s\n", zip_strerror(za));
		return -1;
	}

	/* stat zip size */
	if(!zip_source_is_deleted(zip_src))
	{
		zip_stat_t zst;

		if(zip_source_stat(zip_src, &zst) < 0)
		{
			fprintf(stderr, "can't stat source: %s\n", zip_error_strerror(zip_source_error(zip_src)));
			return -1;
		}
		uncompressed = zst.size;

		zip_source_close(zip_src);
	}

	zip_source_free(zip_src);

	return uncompressed;
}

pod_ssize_t pod_zip_compress(pod_byte_t** dst, pod_byte_t* src, pod_size_t decompressed)
{
}

pod_ssize_t pod_zip_decompress(pod_byte_t** dst, pod_byte_t* src, pod_size_t compressed)
{
	zip_uint64_t uncompressed = 0;

	if(src == NULL)
	{
		fprintf(stderr, "src is a NULL pointer argument!");
		return -1;
	}

	if(dst == NULL)
	{
		fprintf(stderr, "dst is a NULL pointer argument!");
		return -1;
	}

	zip_error_t zip_error;

	/* create source from buffer */
	zip_source_t *zip_src = zip_source_buffer_create(src, compressed, 1, &zip_error);

	if(zip_src == NULL)
	{
		fprintf(stderr, "pod_zip: error creating source!\n");
		zip_error_fini(&zip_error);
		return -1;
	}
	/* open zip archive from source */
	zip_t *za = zip_open_from_source(zip_src, 0, &zip_error);
	if(za == NULL)
	{
		fprintf(stderr, "pod_zip: error opening zip from source!\n");
		zip_error_fini(&zip_error);
		return -1;
	}

	zip_error_fini(&zip_error);

	/* keep the source */
	zip_source_keep(zip_src);

	/* close archive */
	if(zip_close(za) < 0)
	{
		fprintf(stderr, "pod_zip: error closing zip archive of dst buffer: %s\n", zip_strerror(za));
		return -1;
	}

	/* copy new archive to buffer */
	if(zip_source_is_deleted(zip_src))
	{
		/* new archive is empty thus no data */
		dst = NULL;
	}
	else 
	{
		zip_stat_t zst;

		if(zip_source_stat(zip_src, &zst) < 0)
		{
			fprintf(stderr, "can't stat source: %s\n", zip_error_strerror(zip_source_error(zip_src)));
			return -1;
		}
		uncompressed = zst.size;

		
		if(zip_source_open(zip_src) < 0)
		{
			fprintf(stderr, "can't open source: %s\n", zip_error_strerror(zip_source_error(zip_src)));
			return -1;
		}
		if((*dst = calloc(1,uncompressed)) == NULL)
		{
			fprintf(stderr, "calloc failed: %s\n", strerror(errno));
			zip_source_close(zip_src);
			return -1;
		}
		if((zip_uint64_t)zip_source_read(zip_src,*dst,uncompressed) < uncompressed)
		{
			fprintf(stderr, "can't read data from source: %s\n", zip_error_strerror(zip_source_error(zip_src)));
			zip_source_close(zip_src);
			free(*dst);
			return -1;
		}
		zip_source_close(zip_src);
	}

	zip_source_free(zip_src);

	return uncompressed;
}

pod_zip_entry_pod4_t* pod_file_zip_entries_pod4_create(pod_file_pod4_t* pod4)
{
	if(pod4 == NULL)
	{
		fprintf(stderr, "pod_file_zip_entries_pod4_create: error argument pod_file == NULL\n");
		return NULL;
	}
	pod4->zip_entries = calloc(pod4->header->file_count, sizeof(pod_zip_entry_pod4_t));
	if(pod4->zip_entries == NULL)
	{
		fprintf(stderr, "ERROR: Allocate memory for pod_zip_t failed!\n");
		return NULL;
	}

	for(pod_number_t entry = 0; entry < pod4->header->file_count; entry++)
	{
		/* current pod_entry */
		pod_entry_pod4_t* pod_entry = &pod4->entries[entry];
		/* current pod_zip_entry */
		pod_zip_entry_pod4_t* zip_entry = &pod4->zip_entries[entry];

		/* fill zip entry */
		pod4->zip_entries[entry].entry = pod_entry;
		pod4->zip_entries[entry].name = &pod4->path_data[pod_entry->path_offset];

		/* create source from buffer */
		if((zip_entry->src = zip_source_buffer_create(&pod4->entry_data[pod_entry->offset], pod_entry->size, 1, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error creating source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod4->zip_entries);
			return NULL;
		}

		/* open zip archive from source */
		if((zip_entry->za = zip_open_from_source(zip_entry->src, 0, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error opening zip from source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod4->zip_entries);
			return NULL;
		}
		zip_error_fini(&zip_entry->error);

		/* keep the source */
		zip_source_keep(zip_entry->src);

		/* close archive */
		if(zip_close(zip_entry->za) < 0)
		{
			fprintf(stderr, "pod_zip: error closing zip archive '%s': %s\n", zip_entry->name, zip_strerror(zip_entry->za));
		}
	}
	return pod4->zip_entries;
}

pod_zip_entry_pod5_t* pod_file_zip_entries_pod5_create(pod_file_pod5_t* pod5)
{
	if(pod5 == NULL)
	{
		fprintf(stderr, "pod_file_zip_entries_pod5_create: error argument pod_file == NULL\n");
		return NULL;
	}

	pod5->zip_entries = calloc(pod5->header->file_count, sizeof(pod_zip_entry_pod5_t));
	if(pod5->zip_entries == NULL)
	{
		fprintf(stderr, "ERROR: Allocate memory for pod_zip_t failed!\n");
		return NULL;
	}

	for(pod_number_t entry = 0; entry < pod5->header->file_count; entry++)
	{
		/* current pod_entry */
		pod_entry_pod5_t* pod_entry = &pod5->entries[entry];
		/* current pod_zip_entry */
		pod_zip_entry_pod5_t* zip_entry = &pod5->zip_entries[entry];

		/* fill zip entry */
		pod5->zip_entries[entry].entry = pod_entry;
		pod5->zip_entries[entry].name = &pod5->path_data[pod_entry->path_offset];

		/* create source from buffer */
		if((zip_entry->src = zip_source_buffer_create(&pod5->entry_data[pod_entry->offset], pod_entry->size, 1, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error creating source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod5->zip_entries);
			return NULL;
		}

		/* open zip archive from source */
		if((zip_entry->za = zip_open_from_source(zip_entry->src, 0, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error opening zip from source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod5->zip_entries);
			return NULL;
		}
		zip_error_fini(&zip_entry->error);

		/* keep the source */
		zip_source_keep(zip_entry->src);

		/* close archive */
		if(zip_close(zip_entry->za) < 0)
		{
			fprintf(stderr, "pod_zip: error closing zip archive '%s': %s\n", zip_entry->name, zip_strerror(zip_entry->za));
		}
	}
	return pod5->zip_entries;
}

pod_zip_entry_pod6_t* pod_file_zip_entries_pod6_create(pod_file_pod6_t* pod6)
{
	if(pod6 == NULL)
	{
		fprintf(stderr, "pod_file_zip_entries_pod6_create: error argument pod_file == NULL\n");
		return NULL;
	}

	pod6->zip_entries = calloc(pod6->header->file_count, sizeof(pod_zip_entry_pod6_t));
	if(pod6->zip_entries == NULL)
	{
		fprintf(stderr, "ERROR: Allocate memory for pod_zip_t failed!\n");
		return NULL;
	}

	for(pod_number_t entry = 0; entry < pod6->header->file_count; entry++)
	{
		/* current pod_entry */
		pod_entry_pod6_t* pod_entry = &pod6->entries[entry];
		/* current pod_zip_entry */
		pod_zip_entry_pod6_t* zip_entry = &pod6->zip_entries[entry];

		/* fill zip entry */
		pod6->zip_entries[entry].entry = pod_entry;
		pod6->zip_entries[entry].name = &pod6->path_data[pod_entry->path_offset];

		/* create source from buffer */
		if((zip_entry->src = zip_source_buffer_create(&pod6->entry_data[pod_entry->offset], pod_entry->size, 1, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error creating source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod6->zip_entries);
			return NULL;
		}

		/* open zip archive from source */
		if((zip_entry->za = zip_open_from_source(zip_entry->src, 0, &zip_entry->error)) == NULL)
		{
			fprintf(stderr, "pod_zip: error opening zip from source!\n");
			zip_error_fini(&zip_entry->error);
			free(pod6->zip_entries);
			return NULL;
		}
		zip_error_fini(&zip_entry->error);

		/* keep the source */
		zip_source_keep(zip_entry->src);

		/* close archive */
		if(zip_close(zip_entry->za) < 0)
		{
			fprintf(stderr, "pod_zip: error closing zip archive '%s': %s\n", zip_entry->name, zip_strerror(zip_entry->za));
		}
	}
	return pod6->zip_entries;

}
