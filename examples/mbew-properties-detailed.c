#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);

	if(!mbew_status(mbew)) {
		mbew_bool_t b;

		printf("Opened mbew context for '%s'\n", argv[1]);

		printf(" > Duration: %lu\n", mbew_property(mbew, MBEW_PROP_DURATION).ns);
		printf(" > Tracks: %d\n", mbew_property(mbew, MBEW_PROP_TRACKS).num);

		b = mbew_property(mbew, MBEW_PROP_VIDEO).b;

		if(b) printf(
			" > Video: track=%u width=%u height=%u\n",
			mbew_property(mbew, MBEW_PROP_VIDEO_TRACK).num,
			mbew_property(mbew, MBEW_PROP_VIDEO_WIDTH).num,
			mbew_property(mbew, MBEW_PROP_VIDEO_HEIGHT).num
		);

		b = mbew_property(mbew, MBEW_PROP_AUDIO).b;

		if(b) printf(
			" > Audio: track=%u rate=%.2f channels=%u depth=%u\n",
			mbew_property(mbew, MBEW_PROP_AUDIO_TRACK).num,
			mbew_property(mbew, MBEW_PROP_AUDIO_RATE).hz,
			mbew_property(mbew, MBEW_PROP_AUDIO_CHANNELS).num,
			mbew_property(mbew, MBEW_PROP_AUDIO_DEPTH).num
		);
	}

	else printf("Error creating mbew context (status: %d)\n", mbew_status(mbew));

	mbew_destroy(mbew);

	return 0;
}

