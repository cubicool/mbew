#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	printf("MBEW_SRC_FILE: %s\n", mbew_string(MBEW_TYPE_SRC, MBEW_SRC_FILE));
	printf("MBEW_STATUS_SUCCESS: %s\n", mbew_string(MBEW_TYPE_STATUS, MBEW_STATUS_SUCCESS));
	printf("MBEW_PROP_DURATION: %s\n", mbew_string(MBEW_TYPE_PROP, MBEW_PROP_DURATION));
	printf("MBEW_PROP_AUDIO_DEPTH: %s\n", mbew_string(MBEW_TYPE_PROP, MBEW_PROP_AUDIO_DEPTH));
	printf("MBEW_TRUE: %s\n", mbew_string(MBEW_TYPE_BOOL, MBEW_TRUE));
	printf("MBEW_DATA_AUDIO: %s\n", mbew_string(MBEW_TYPE_DATA, MBEW_DATA_AUDIO));

	return 0;
}

