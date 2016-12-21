#include "mbew.h"

#include <stdio.h>

void video_cairo_callback(mbew_iter_t type, mbew_num_t index, mbew_ns_t tstamp, const void* data) {
	if(type == MBEW_ITER_VIDEO) {
		printf("index: %u // tstamp: %lu\n", index, tstamp);

		/* const mbew_data_video_t* video_data = (const mbew_data_video_t*)(data); */
	}
}

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);

	if(!mbew_status(mbew)) {
		printf("Opened mbew context for '%s'\n", argv[1]);

		mbew_iterate(mbew, video_cairo_callback);
	}

	else printf("Error creating mbew context (status: %d)\n", mbew_status(mbew));

	mbew_destroy(mbew);

	return 0;
}

