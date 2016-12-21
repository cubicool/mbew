#include "mbew-private.h"

#include <stdio.h>
#include <string.h>

static int mbew_file_read(void* p, size_t length, void* file) {
	size_t r;
	FILE* fp = file;

	r = fread(p, length, 1, fp);

	if(!r && feof(fp)) return 0;

	return r == 0 ? -1 : 1;
}

static int mbew_file_seek(int64_t offset, int whence, void * file) {
	FILE* fp = file;

	return fseek(fp, offset, whence);
}

static int64_t mbew_file_tell(void * fp) {
	return ftell(fp);
}

static int mbew_memory_read(void* p, size_t length, void* memory) {
	return -1;
}

static int mbew_memory_seek(int64_t offset, int whence, void * memory) {
	return -1;
}

static int64_t mbew_memory_tell(void * fp) {
	return -1;
}

static nestegg_io mbew_file_io = {
	mbew_file_read,
	mbew_file_seek,
	mbew_file_tell,
	NULL
};

static nestegg_io mbew_memory_io = {
	mbew_memory_read,
	mbew_memory_seek,
	mbew_memory_tell,
	NULL
};

mbew_bool_t mbew_create_src_file(mbew_t* mbew, void* data) {
	memcpy(&mbew->ne_io, &mbew_file_io, sizeof(nestegg_io));

	mbew->ne_io.userdata = fopen((const char*)(data), "rb");

	if(!mbew->ne_io.userdata) return MBEW_FALSE;

	return MBEW_TRUE;
}

mbew_bool_t mbew_create_src_memory(mbew_t* mbew, void* data) {
	memcpy(&mbew->ne_io, &mbew_memory_io, sizeof(nestegg_io));

	return MBEW_FALSE;
}

