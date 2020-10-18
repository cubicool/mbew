#include "mbew.h"

#include <ao/ao.h>
#include <string.h>

int main(int argc, char** argv) {
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		ao_device* device = NULL;
		ao_sample_format format;

		int driver;

		ao_initialize();

		driver = ao_default_driver_id();

		memset(&format, 0, sizeof(ao_sample_format));

		format.bits = 16;
		format.channels = mbew_property(m, MBEW_PROPERTY_AUDIO_CHANNELS).num;
		format.rate = mbew_property(m, MBEW_PROPERTY_AUDIO_RATE).hz;
		format.byte_format = AO_FMT_LITTLE;

		device = ao_open_live(driver, &format, NULL);

		if(!device) printf("Failed to acquire audio device/format.\n");

		else {
			while(mbew_iterate(m, MBEW_ITERATE_AUDIO)) ao_play(
				device,
				(char*)(mbew_iter_pcm16(m)),
				mbew_iter_pcm16_size(m) * 2
			);

			ao_close(device);
		}

		ao_shutdown();
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}

