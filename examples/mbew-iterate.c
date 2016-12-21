#include "mbew.h"

#include <stdio.h>

void iterate_callback(mbew_iter_t type, mbew_num_t index, mbew_ns_t tstamp, void* data) {
	printf(
		"packet=%03u type=%s tstamp=%4.2fs\n",
		index,
		mbew_string(MBEW_TYPE_ITER, type),
		((double)(tstamp / 1000000)) / 1000.0
	);
}

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(mbew))) mbew_iterate(mbew, iterate_callback);

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(mbew);

	return 0;
}

