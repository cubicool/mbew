#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	FILE* audio = NULL;
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		audio = fopen("audio.pcm", "wb+");

		while(mbew_iterate(m, MBEW_ITERATE_AUDIO)) {
			mbew_num_t index = mbew_iter_index(m);
			mbew_data_t type = mbew_iter_type(m);
			mbew_ns_t timestamp = mbew_iter_timestamp(m);

			const int16_t* pcm16 = mbew_iter_pcm16(m);
			mbew_num_t size = mbew_iter_pcm16_size(m);

			printf(
				"index=%03u type=%s timestamp=%4.2fs data.size=%u\n",
				index,
				mbew_string(type),
				((double)(timestamp / 1000000)) / 1000.0,
				size
			);

			fwrite(pcm16, size, 2, audio);

			/* if(!mbew_valid(m)) {
				printf("Error during iteration: %s\n", mbew_string(mbew_status(m)));

				break;
			} */
		}
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	fclose(audio);

	return 0;
}

