#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	printf("MBEW_SOURCE_FILE: %s\n", mbew_string(MBEW_SOURCE_FILE));
	printf("MBEW_STATUS_VALID: %s\n", mbew_string(MBEW_STATUS_VALID));
	printf("MBEW_STATUS_GET_FRAME: %s\n", mbew_string(MBEW_STATUS_GET_FRAME));
	printf("MBEW_PROPERTY_DURATION: %s\n", mbew_string(MBEW_PROPERTY_DURATION));
	printf("MBEW_PROPERTY_AUDIO_DEPTH: %s\n", mbew_string(MBEW_PROPERTY_AUDIO_DEPTH));
	printf("MBEW_TRUE: %s\n", mbew_string(MBEW_TRUE));
	printf("MBEW_DATA_AUDIO: %s\n", mbew_string(MBEW_DATA_AUDIO));

	return 0;
}

