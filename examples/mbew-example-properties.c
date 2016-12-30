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
		mbew_bool_t b;

		printf("Opened context for '%s'\n", argv[1]);

		printf(" > Duration: %luns\n", mbew_property(m, MBEW_PROPERTY_DURATION).ns);
		printf(" > Scale: %luns\n", mbew_property(m, MBEW_PROPERTY_SCALE).ns);
		printf(" > Tracks: %d\n", mbew_property(m, MBEW_PROPERTY_TRACKS).num);

		b = mbew_property(m, MBEW_PROPERTY_VIDEO).b;

		if(b) printf(
			" > Video: track=%u codec=%s width=%u height=%u\n",
			mbew_property(m, MBEW_PROPERTY_VIDEO_TRACK).num,
			mbew_string(mbew_property(m, MBEW_PROPERTY_VIDEO_CODEC).num),
			mbew_property(m, MBEW_PROPERTY_VIDEO_WIDTH).num,
			mbew_property(m, MBEW_PROPERTY_VIDEO_HEIGHT).num
		);

		b = mbew_property(m, MBEW_PROPERTY_AUDIO).b;

		if(b) printf(
			" > Audio: track=%u codec=%s rate=%.2f channels=%u depth=%u\n",
			mbew_property(m, MBEW_PROPERTY_AUDIO_TRACK).num,
			mbew_string(mbew_property(m, MBEW_PROPERTY_AUDIO_CODEC).num),
			mbew_property(m, MBEW_PROPERTY_AUDIO_RATE).hz,
			mbew_property(m, MBEW_PROPERTY_AUDIO_CHANNELS).num,
			mbew_property(m, MBEW_PROPERTY_AUDIO_DEPTH).num
		);
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}

