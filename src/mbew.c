#include "mbew-private.h"

#include <stdlib.h>

mbew_t mbew_create(mbew_source_t src, ...) {
	mbew_t m = (mbew_t)(calloc(1, sizeof(struct _mbew_t)));
	va_list args;

	/* If calloc() fails for some reason, return NULL. */
	if(!m) return NULL;

	va_start(args, src);

	if(src == MBEW_SOURCE_WRITE_FILE) {
		const char* path = va_arg(args, const char*);
		const mbew_write_config_t* config = va_arg(args, const mbew_write_config_t*);

		m->src = src;

		if(mbew_writer_create(m, path, config)) m->status = MBEW_STATUS_VALID;
	}

	else mbew_reader_create(src, m, args);

	va_end(args);

	return m;
}

void mbew_destroy(mbew_t m) {
	if(!m) return;

	if(m->src == MBEW_SOURCE_WRITE_FILE) mbew_writer_destroy(m);

	else mbew_reader_destroy(m);
}

mbew_bool_t mbew_reset(mbew_t m) {
	if(!m || m->src == MBEW_SOURCE_WRITE_FILE) {
		if(m) m->status = MBEW_STATUS_NOT_IMPLEMENTED;

		return MBEW_FALSE;
	}

	return mbew_reader_reset(m);
}

mbew_status_t mbew_status(mbew_t m) {
	return !m ? MBEW_STATUS_NULL_CONTEXT : m->status;
}

mbew_propval_t mbew_property(mbew_t m, ...) {
	mbew_propval_t r = { 0 };
	mbew_property_t prop;
	va_list args;

	if(!m) return r;

	va_start(args, m);
	prop = va_arg(args, mbew_property_t);
	va_end(args);

	if(m->src == MBEW_SOURCE_WRITE_FILE) return mbew_writer_property(m, prop);

	return mbew_reader_property(m, prop);
}
