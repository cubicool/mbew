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

