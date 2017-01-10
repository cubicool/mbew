#include "mbew-private.h"

typedef struct _mbew_string_t {
	const mbew_num_t count;
	const char* value[];
} mbew_string_t;

static mbew_string_t MBEW_STRING_SOURCE = { 2, {
	"SOURCE_FILE",
	"SOURCE_MEMORY"
}};

static mbew_string_t MBEW_STRING_STATUS = { 29, {
	"MBEW_STATUS_VALID",
	"MBEW_STATUS_NOT_IMPLEMENTED",
	"MBEW_STATUS_NULL_CONTEXT",
	"MBEW_STATUS_ITERATE_FLAGS",
	"MBEW_STATUS_SOURCE_FILE",
	"MBEW_STATUS_SOURCE_MEMORY",
	"MBEW_STATUS_NESTEGG_INIT",
	"MBEW_STATUS_NESTEGG_DURATION",
	"MBEW_STATUS_NESTEGG_TSTAMP_SCALE",
	"MBEW_STATUS_NESTEGG_TRACK_COUNT",
	"MBEW_STATUS_NESTEGG_TRACK_TYPE",
	"MBEW_STATUS_NESTEGG_TRACK_VIDEO_PARAMS",
	"MBEW_STATUS_NESTEGG_TRACK_AUDIO_PARAMS",
	"MBEW_STATUS_NESTEGG_TRACK_CODEC_DATA_COUNT",
	"MBEW_STATUS_NESTEGG_TRACK_CODEC_DATA",
	"MBEW_STATUS_NESTEGG_TRACK_SEEK",
	"MBEW_STATUS_NESTEGG_OFFSET_SEEK",
	"MBEW_STATUS_NESTEGG_PACKET_TRACK",
	"MBEW_STATUS_NESTEGG_PACKET_COUNT",
	"MBEW_STATUS_NESTEGG_PACKET_TSTAMP",
	"MBEW_STATUS_NESTEGG_PACKET_DATA",
	"MBEW_STATUS_NESTEGG_READ_PACKET",
	"MBEW_STATUS_VPX_CODEC_DEC_INIT",
	"MBEW_STATUS_VPX_CODEC_DECODE",
	"MBEW_STATUS_VPX_CODEC_GET_FRAME",
	"MBEW_STATUS_VORBIS_HEADER_COUNT",
	"MBEW_STATUS_VORBIS_SYNTHESIS_HEADERIN",
	"MBEW_STATUS_VORBIS_SYNTHESIS_INIT",
	"MBEW_STATUS_VORBIS_BLOCK_INIT",
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

