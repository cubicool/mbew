#include "mbew-private.h"

typedef struct _mbew_string_t {
	const mbew_num_t count;
	const char* value[];
} mbew_string_t;

static mbew_string_t MBEW_STRING_SOURCE = { 2, {
	"SOURCE_FILE",
	"SOURCE_MEMORY"
}};

static mbew_string_t MBEW_STRING_STATUS = { 27, {
	"STATUS_VALID",
	"STATUS_NULL_CONTEXT",
	"STATUS_SOURCE_FILE",
	"STATUS_SOURCE_MEMORY",
	"STATUS_INIT_IO",
	"STATUS_INIT_CODEC",
	"STATUS_DURATION",
	"STATUS_SCALE",
	"STATUS_TRACK_COUNT",
	"STATUS_UNKNOWN_TRACK",
	"STATUS_PARAMS_VIDEO",
	"STATUS_PARAMS_AUDIO",
	"STATUS_CODEC_DATA_COUNT",
	"STATUS_CODEC_DATA",
	"STATUS_PACKET_READ",
	"STATUS_PACKET_TRACK",
	"STATUS_PACKET_COUNT",
	"STATUS_PACKET_TSTAMP",
	"STATUS_PACKET_DURATION",
	"STATUS_PACKET_DATA",
	"STATUS_VPX_DECODE",
	"STATUS_GET_FRAME",
	"STATUS_SEEK_OFFSET",
	"STATUS_SEEK_VIDEO",
	"STATUS_SEEK_AUDIO",
	"STATUS_ITERATE_FLAGS",
	"STATUS_NOT_IMPLEMENTED"
}};

static mbew_string_t MBEW_STRING_PROPERTY = { 14, {
	"PROPERTY_DURATION",
	"PROPERTY_SCALE",
	"PROPERTY_TRACKS",
	"PROPERTY_VIDEO",
	"PROPERTY_VIDEO_TRACK",
	"PROPERTY_VIDEO_CODEC",
	"PROPERTY_VIDEO_WIDTH",
	"PROPERTY_VIDEO_HEIGHT",
	"PROPERTY_AUDIO",
	"PROPERTY_AUDIO_TRACK",
	"PROPERTY_AUDIO_CODEC",
	"PROPERTY_AUDIO_RATE",
	"PROPERTY_AUDIO_CHANNELS",
	"PROPERTY_AUDIO_DEPTH"
}};

static mbew_string_t MBEW_STRING_DATA = { 3, {
	"DATA_SYNC",
	"DATA_VIDEO",
	"DATA_AUDIO"
}};

static mbew_string_t MBEW_STRING_CODEC = { 4, {
	"CODEC_VP8",
	"CODEC_VORBIS",
	"CODEC_VP9",
	"CODEC_OPUS"
}};

static mbew_string_t* MBEW_STRING[] = {
	&MBEW_STRING_SOURCE,
	&MBEW_STRING_STATUS,
	&MBEW_STRING_PROPERTY,
	&MBEW_STRING_DATA,
	&MBEW_STRING_CODEC
};

const char* mbew_string(mbew_num_t e) {
	if(e == MBEW_FALSE) return "FALSE";

	else if(e == MBEW_TRUE) return "TRUE";

	else {
		mbew_num_t id = mbew_enum_id(e);
		mbew_num_t value = mbew_enum_value(e);

		if(!mbew_enum_valid(e)) return NULL;

		else if(id == MBEW_ID_ITERATE) return "!!TODO!!";

		else {
			if(value >= MBEW_STRING[id - 1]->count) return NULL;

			return MBEW_STRING[id - 1]->value[value];
		}
	}
}

