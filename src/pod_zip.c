#include "pod_zip.h"


pod_zip_entry_pod4_t* pod_file_zip_entries_pod4_create(pod_file_pod4_t* pod4)
{
	if(pod_file == NULL)
	{
		fprintf(stderr, "pod_file_zip_entries_pod4_create: error argument pod_file == NULL\n");
		return NULL;
	}
	pod4->zip_entries = calloc(pod4->header.file_count, sizeof(pod_zip_entry_pod4_t));
	if(pod4->zip_entries == NULL)
	{
		fprintf(stderr, "ERROR: Allocate memory for pod_zip_t failed!\n");
		return NULL;
	}

	for(pod_number_t entry = 0; entry < pod_file->header.file_count; entry++)
	{
		/* current pod_entry */
		pod_entry_pod4_t* pod_entry = &pod4->entries[entry];
		/* current pod_zip_entry */
		pod_zip_entry_pod4_t* zip_entry = &pod4->zip_entries[entry];

		/* fill zip entry */
		pod4->zip_entries[entry].entry = *pod_entry;
		pod4->zip_entries[entry].name = pod4->path_data[pod_entry->path_offset];

	/* create source from buffer */
	if((zip_entry->src = zip_source_buffer_create(pod_file->entry_data[pod_entry->offset], pod_entry->size, 1, &zip_entry->error)) == NULL)
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
		fprintf(stderr, "pod_zip: error closing zip archive '%s': %s\n", zip_entry->name., zip_strerror(zip_entry->za));
	}
	return pod4->zip_entries;
}
