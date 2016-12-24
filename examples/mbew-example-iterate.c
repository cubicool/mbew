#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	mbew_t m = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(m))) {
		while(mbew_iterate(m, 0)) {
			mbew_num_t index = mbew_iter_index(m);
			mbew_data_t type = mbew_iter_type(m);
			mbew_ns_t timestamp = mbew_iter_timestamp(m);

			printf(
				"index=%03u type=%s timestamp=%4.2fs\n",
				index,
				mbew_string(MBEW_TYPE_DATA, type),
				((double)(timestamp / 1000000)) / 1000.0
			);
		}
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(m);

	return 0;
}

