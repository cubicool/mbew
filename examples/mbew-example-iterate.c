#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		while(mbew_iterate(m, 0)) {
			mbew_num_t index = mbew_iter_index(m);
			mbew_data_t type = mbew_iter_type(m);
			mbew_ns_t timestamp = mbew_iter_timestamp(m);

			printf(
				"index=%03u type=%s timestamp=%4.2fs\n",
				index,
				mbew_string(type),
				((double)(timestamp / 1000000)) / 1000.0
			);

			if(!mbew_valid(m)) {
				printf("Error during iteration: %s\n", mbew_string(mbew_status(m)));

				break;
			}
		}
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}

