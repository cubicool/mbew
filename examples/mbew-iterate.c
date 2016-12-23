#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(mbew))) {
		mbew_iter_t* iter = NULL;
		mbew_num_t i;

		for(i = 0; i < 3; i++) {
			printf("ITERATION: %u\n", i);

			while(mbew_iterate(mbew, &iter, 0)) {
				mbew_num_t index = mbew_iter_index(iter);
				mbew_data_t type = mbew_iter_type(iter);
				mbew_ns_t timestamp = mbew_iter_timestamp(iter);

				printf(
					"index=%03u type=%s timestamp=%4.2fs\n",
					index,
					mbew_string(MBEW_TYPE_DATA, type),
					((double)(timestamp / 1000000)) / 1000.0
				);
			}

			if(!mbew_reset(mbew)) printf(
				"Couldn't reset: %s\n",
				mbew_string(MBEW_TYPE_STATUS, mbew_status(mbew))
			);
		}

		mbew_iter_destroy(iter);
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(mbew);

	return 0;
}

