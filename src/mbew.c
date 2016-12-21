#include "mbew-private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MBEW_RETURN(st) { mbew->status = MBEW_STATUS_##st; return mbew; }

mbew_t* mbew_create(mbew_src_t src, void* data) {
	mbew_t* mbew = (mbew_t*)(calloc(1, sizeof(mbew_t)));

	mbew_num_t i;

	/* If calloc() fails for some reason, return NULL. */
	if(!mbew) return NULL;

	/* Call the appropriate src setup function. */
	if(src != MBEW_SRC_FILE) MBEW_RETURN(NOT_IMPLEMENTED);

	if(!mbew_create_src_file(mbew, data)) MBEW_RETURN(SRC_FILE);

	/* Regardless of the src, begin calling the "common" setup routines. */
	if(nestegg_init(&mbew->ne, mbew->ne_io, NULL, -1)) MBEW_RETURN(INIT_IO);
	if(nestegg_duration(mbew->ne, &mbew->duration)) MBEW_RETURN(DURATION);
	if(nestegg_tstamp_scale(mbew->ne, &mbew->scale)) MBEW_RETURN(SCALE);
	if(nestegg_track_count(mbew->ne, &mbew->tracks)) MBEW_RETURN(TRACK_COUNT);

	/* Iterate through all the tracks, latching on to the first audio and video streams found. */
	for(i = 0; i < mbew->tracks; i++) {
		mbew_track_t* track = NULL;

		int type = nestegg_track_type(mbew->ne, i);
		int codec = nestegg_track_codec_id(mbew->ne, i);

		if(type == NESTEGG_TRACK_VIDEO) track = &mbew->video.track;

		else if(type == NESTEGG_TRACK_AUDIO) track = &mbew->audio.track;

		else MBEW_RETURN(UNKNOWN_TRACK);

		/* If a previous audio/video track was found, ignore the current one.
		 * TODO: Expand the API so that it can handle multiple tracks of the same type; I couldn't
		 * find a sample WebM video with more than 2 tracks, however. */
		if(track->init) continue;

		track->type = type;
		track->codec = codec;
		track->index = i;
		track->init = 1;

		if(type == NESTEGG_TRACK_VIDEO) {
			mbew->video.iface = codec == NESTEGG_CODEC_VP9 ?
				&vpx_codec_vp9_dx_algo :
				&vpx_codec_vp8_dx_algo
			;

			if(vpx_codec_dec_init(
				&mbew->video.codec,
				mbew->video.iface,
				NULL,
				mbew->flags
			)) MBEW_RETURN(INIT_CODEC);

			if(nestegg_track_video_params(
				mbew->ne,
				i,
				&mbew->video.params
			)) MBEW_RETURN(PARAMS_VIDEO);
		}

		/* The if/else if/else above will make sure we're either audio or video; never unknown. */
		else {
			if(nestegg_track_audio_params(
				mbew->ne,
				i,
				&mbew->audio.params
			)) MBEW_RETURN(PARAMS_AUDIO);
		}
	}

	return mbew;
}

void mbew_destroy(mbew_t* mbew) {
	if(mbew->src == MBEW_SRC_FILE && mbew->ne_io.userdata) fclose(mbew->ne_io.userdata);

	if(mbew->video.track.init) vpx_codec_destroy(&mbew->video.codec);

	if(mbew->ne) nestegg_destroy(mbew->ne);

	free(mbew);
}

mbew_status_t mbew_status(mbew_t* mbew) {
	return !mbew ? MBEW_STATUS_NULL : mbew->status;
}

#define CASE_PROPERTY(prop, ty, attr) case MBEW_PROP_##prop: r.ty = mbew->attr; break

mbew_prop_val_t mbew_property(mbew_t* mbew, ...) {
	mbew_prop_val_t r = { 0 };
	mbew_prop_t prop;

	va_list arg_enum;
	va_list arg_str;

	va_start(arg_enum, mbew);
	va_copy(arg_str, arg_enum);

	prop = va_arg(arg_enum, mbew_prop_t);

	/* TODO: Handle string value. */
	/* if(prop >= MBEW_PROP_MAX) {
		const char* str = va_arg(arg_str, const char*);
	} */

	va_end(arg_str);
	va_end(arg_enum);

	switch(prop) {
		CASE_PROPERTY(DURATION, ns, duration);
		CASE_PROPERTY(SCALE, ns, scale);
		CASE_PROPERTY(TRACKS, num, tracks);
		CASE_PROPERTY(VIDEO, b, video.track.init);
		CASE_PROPERTY(VIDEO_TRACK, num, video.track.index);
		CASE_PROPERTY(VIDEO_WIDTH, num, video.params.width);
		CASE_PROPERTY(VIDEO_HEIGHT, num, video.params.height);
		CASE_PROPERTY(AUDIO, b, audio.track.init);
		CASE_PROPERTY(AUDIO_TRACK, num, audio.track.index);
		CASE_PROPERTY(AUDIO_RATE, hz, audio.params.rate);
		CASE_PROPERTY(AUDIO_CHANNELS, num, audio.params.channels);
		CASE_PROPERTY(AUDIO_DEPTH, num, audio.params.depth);

		default:
			/* TODO: Set error status. */
			break;
	}

	return r;
}

void mbew_properties(mbew_t* mbew, ...) {
	/* TODO: Handle a variable number of properties, followed by pointers to be filled with the
	 * corresponding values. */
}

static const char* STRINGS[] = {
	/* Offset: 0 */
	"MBEW_SRC_FILE",
	"MBEW_SRC_MEMORY",

	/* Offset: 2 */
	"MBEW_STATUS_SUCCESS",
	"MBEW_STATUS_NULL",
	"MBEW_STATUS_SRC_FILE",
	"MBEW_STATUS_SRC_MEMORY",
	"MBEW_STATUS_INIT_IO",
	"MBEW_STATUS_INIT_CODEC",
	"MBEW_STATUS_DURATION",
	"MBEW_STATUS_SCALE",
	"MBEW_STATUS_TRACK_COUNT",
	"MBEW_STATUS_UNKNOWN_TRACK",
	"MBEW_STATUS_PARAMS_VIDEO",
	"MBEW_STATUS_PARAMS_AUDIO",
	"MBEW_STATUS_PACKET_TRACK",
	"MBEW_STATUS_PACKET_COUNT",
	"MBEW_STATUS_PACKET_TSTAMP",
	"MBEW_STATUS_PACKET_DURATION",
	"MBEW_STATUS_PACKET_DATA",
	"MBEW_STATUS_VPX_DECODE",
	"MBEW_STATUS_TODO",
	"MBEW_STATUS_NOT_IMPLEMENTED",

	/* Offset: 22 */
	"MBEW_PROP_DURATION",
	"MBEW_PROP_SCALE",
	"MBEW_PROP_TRACKS",
	"MBEW_PROP_VIDEO",
	"MBEW_PROP_VIDEO_TRACK",
	"MBEW_PROP_VIDEO_WIDTH",
	"MBEW_PROP_VIDEO_HEIGHT",
	"MBEW_PROP_AUDIO",
	"MBEW_PROP_AUDIO_TRACK",
	"MBEW_PROP_AUDIO_RATE",
	"MBEW_PROP_AUDIO_CHANNELS",
	"MBEW_PROP_AUDIO_DEPTH",

	/* Offset: 34 */
	"MBEW_FALSE",
	"MBEW_TRUE",

	/* Offset: 36 */
	"MBEW_DATA_VIDEO",
	"MBEW_DATA_AUDIO"
};

#define OFFSET_SRC 0
#define OFFSET_STATUS 2
#define OFFSET_PROP 22
#define OFFSET_BOOL 34
#define OFFSET_DATA 36
#define OFFSET_MAX 38

#define VAL_SRC 2
#define VAL_STATUS 20
#define VAL_PROP 12
#define VAL_BOOL 2
#define VAL_DATA 2

#define CASE_TYPE(ty) case MBEW_TYPE_##ty: if(val < VAL_##ty) { offset = OFFSET_##ty; } break

const char* mbew_string(mbew_type_t type, ...) {
	va_list arg;

	mbew_num_t val = 0;
	mbew_num_t offset = OFFSET_MAX;

	va_start(arg, type);

	val = va_arg(arg, mbew_num_t);

	switch(type) {
		CASE_TYPE(SRC);
		CASE_TYPE(STATUS);
		CASE_TYPE(PROP);
		CASE_TYPE(BOOL);
		CASE_TYPE(DATA);

		default:
			break;
	}

	va_end(arg);

	if(offset < OFFSET_MAX) return STRINGS[offset + val];

	return "ERROR";
}

