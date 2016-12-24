#include "mbew-private.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int mbew_file_read(void* dest, size_t length, void* userdata) {
	size_t r;
	FILE* fp = (FILE*)(userdata);

	r = fread(dest, length, 1, fp);

	if(!r && feof(fp)) return 0;

	return r == 0 ? -1 : 1;
}

static int mbew_file_seek(int64_t offset, int whence, void* userdata) {
	return fseek((FILE*)(userdata), offset, whence);
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

static mbew_bool_t mbew_src_file_create(mbew_t* mbew, va_list args) {
	const char* path = va_arg(args, const char*);

	if(!path) return MBEW_FALSE;

	memcpy(&mbew->ne_io, &MBEW_IO_FILE, sizeof(nestegg_io));

	mbew->ne_io.userdata = fopen(path, "rb");

	if(!mbew->ne_io.userdata) return MBEW_FALSE;

	return MBEW_TRUE;
}

static void mbew_src_file_destroy(mbew_t* mbew) {
	if(mbew->ne_io.userdata) fclose(mbew->ne_io.userdata);
}

typedef struct _mbew_memory_t {
	void* data;
	int64_t size;
	int64_t pos;
} mbew_memory_t;

static int mbew_memory_read(void* dest, size_t length, void* userdata) {
	mbew_memory_t* mem = (mbew_memory_t*)(userdata);

	memcpy(dest, mem->data + mem->pos, length);

	mem->pos += length;

	if(mem->pos == mem->size) return 0;

	return 1;
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

static mbew_bool_t mbew_src_memory_create(mbew_t* mbew, va_list args) {
	mbew_memory_t* mem = (mbew_memory_t*)(calloc(1, sizeof(mbew_memory_t)));

	if(!mem) return MBEW_FALSE;

	mem->data = va_arg(args, void*);
	mem->size = va_arg(args, size_t);
	mem->pos = 0;

	if(!mem->data || mem->size <= 0) return MBEW_FALSE;

	memcpy(&mbew->ne_io, &MBEW_IO_MEMORY, sizeof(nestegg_io));

	mbew->ne_io.userdata = mem;

	return MBEW_TRUE;
}

static void mbew_src_memory_destroy(mbew_t* mbew) {
}

mbew_bool_t mbew_src_create(mbew_src_t src, mbew_t* mbew, va_list args) {
	if(
		src == MBEW_SRC_FILE &&
		!mbew_src_file_create(mbew, args)
	) mbew->status = MBEW_STATUS_SRC_FILE;

	else if(
		src == MBEW_SRC_MEMORY &&
		!mbew_src_memory_create(mbew, args)
	) mbew->status = MBEW_STATUS_SRC_MEMORY;

	return !mbew->status;
}

void mbew_src_destroy(mbew_t* mbew) {
	if(mbew->src == MBEW_SRC_FILE) mbew_src_file_destroy(mbew);

	else if(mbew->src == MBEW_SRC_MEMORY) mbew_src_memory_destroy(mbew);
}

