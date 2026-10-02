#include "mbew-private.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int64_t mbew_file_read(void* dest, size_t length, void* userdata) {
	FILE* fp = (FILE*)(userdata);
	size_t r = fread(dest, 1, length, fp);

	if(r == 0 && ferror(fp)) return -1;

	return (int64_t)(r);
}

static int mbew_file_seek(int64_t offset, int whence, void* userdata) {
	return fseek((FILE*)(userdata), (long)(offset), whence);
}

static int64_t mbew_file_tell(void* userdata) {
	return ftell((FILE*)(userdata));
}

static nestegg_io MBEW_IO_FILE = {
	mbew_file_read,
	mbew_file_seek,
	mbew_file_tell,
	NULL
};

static mbew_bool_t mbew_src_file_create(mbew_t m, va_list args) {
	const char* path = va_arg(args, const char*);

	if(!path) return MBEW_FALSE;

	memcpy(&m->ne_io, &MBEW_IO_FILE, sizeof(nestegg_io));

	m->ne_io.userdata = fopen(path, "rb");

	if(!m->ne_io.userdata) return MBEW_FALSE;

	return MBEW_TRUE;
}

static void mbew_src_file_destroy(mbew_t m) {
	if(m->ne_io.userdata) fclose(m->ne_io.userdata);
}

typedef struct _mbew_memory_t {
	mbew_bytes_t data;

	int64_t size;
	int64_t pos;
} mbew_memory_t;

static int64_t mbew_memory_read(void* dest, size_t length, void* userdata) {
	mbew_memory_t* mem = (mbew_memory_t*)(userdata);
	int64_t remaining = mem->size - mem->pos;
	int64_t n = (int64_t)(length) < remaining ? (int64_t)(length) : remaining;

	if(n <= 0) return 0;

	memcpy(dest, mem->data + mem->pos, (size_t)(n));

	mem->pos += n;

	return n;
}

static int mbew_memory_seek(int64_t offset, int whence, void* userdata) {
	mbew_memory_t* mem = (mbew_memory_t*)(userdata);

	if(whence == NESTEGG_SEEK_SET) mem->pos = offset;

	else if(whence == NESTEGG_SEEK_CUR) mem->pos += offset;

	else mem->pos = mem->size - offset;

	return 0;
}

static int64_t mbew_memory_tell(void* userdata) {
	mbew_memory_t* mem = (mbew_memory_t*)(userdata);

	return mem->pos;
}

static nestegg_io MBEW_IO_MEMORY = {
	mbew_memory_read,
	mbew_memory_seek,
	mbew_memory_tell,
	NULL
};

static mbew_bool_t mbew_src_memory_create(mbew_t m, va_list args) {
	mbew_memory_t* mem = (mbew_memory_t*)(calloc(1, sizeof(mbew_memory_t)));
	size_t size;

	if(!mem) return MBEW_FALSE;

	mem->data = va_arg(args, void*);
	size = va_arg(args, size_t);

	if(size > INT64_MAX) {
		free(mem);

		return MBEW_FALSE;
	}

	mem->size = (int64_t)size;
	mem->pos = 0;

	if(!mem->data || mem->size <= 0) return MBEW_FALSE;

	memcpy(&m->ne_io, &MBEW_IO_MEMORY, sizeof(nestegg_io));

	m->ne_io.userdata = mem;

	return MBEW_TRUE;
}

static void mbew_src_memory_destroy(mbew_t m) {
	free(m->ne_io.userdata);
}

mbew_bool_t mbew_src_create(mbew_source_t src, mbew_t m, va_list args) {
	if(
		src == MBEW_SOURCE_FILE &&
		!mbew_src_file_create(m, args)
	) m->status = MBEW_STATUS_SOURCE_FILE;

	else if(
		src == MBEW_SOURCE_MEMORY &&
		!mbew_src_memory_create(m, args)
	) m->status = MBEW_STATUS_SOURCE_MEMORY;

	else m->src = src;

	return !m->status;
}

void mbew_src_destroy(mbew_t m) {
	if(m->src == MBEW_SOURCE_FILE) mbew_src_file_destroy(m);

	else if(m->src == MBEW_SOURCE_MEMORY) mbew_src_memory_destroy(m);
}
