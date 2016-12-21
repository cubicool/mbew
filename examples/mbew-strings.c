#include "mbew.h"

#include <stdio.h>

int main(int argc, char** argv) {
	printf("SRC.FILE: %s\n", mbew_string(MBEW_TYPE_SRC, MBEW_SRC_FILE));

	printf("STATUS.SUCCESS: %s\n", mbew_string(MBEW_TYPE_STATUS, MBEW_STATUS_SUCCESS));
	printf("STATUS.TODO: %s\n", mbew_string(MBEW_TYPE_STATUS, MBEW_STATUS_TODO));

	printf("PROP.DURATION: %s\n", mbew_string(MBEW_TYPE_PROP, MBEW_PROP_DURATION));
	printf("PROP.AUDIO_DEPTH: %s\n", mbew_string(MBEW_TYPE_PROP, MBEW_PROP_AUDIO_DEPTH));

	printf("BOOL.TRUE: %s\n", mbew_string(MBEW_TYPE_BOOL, MBEW_TRUE));

	printf("ITER.AUDIO: %s\n", mbew_string(MBEW_TYPE_DATA, MBEW_DATA_AUDIO));

	return 0;
}

