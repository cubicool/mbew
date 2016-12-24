#include "mbew-private.h"

#include <stdlib.h>
#include <string.h>

static void mbew_log(nestegg* ne, mbew_num_t severity, const char* format, ...) {
#if 0
	va_list args;

	va_start(args, format);

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

	va_end(args);
#endif
}

#define MBEW_RETURN(st) { m->status = MBEW_STATUS_##st; return m; }

mbew_t mbew_create(mbew_src_t src, ...) {
	mbew_t m = (mbew_t)(calloc(1, sizeof(struct _mbew_t)));

	mbew_num_t i;

	va_list args;

	/* If calloc() fails for some reason, return NULL. */
	if(!m) return NULL;

	va_start(args, src);

	/* mbew_src_create() will properly set the error status, if any. */
	if(!mbew_src_create(src, m, args)) return NULL;

	va_end(args);

	/* Regardless of the src, begin calling the "common" setup routines. */
	if(nestegg_init(&m->ne, m->ne_io, mbew_log, -1)) MBEW_RETURN(INIT_IO);
	if(nestegg_duration(m->ne, &m->duration)) MBEW_RETURN(DURATION);
	if(nestegg_tstamp_scale(m->ne, &m->scale)) MBEW_RETURN(SCALE);
	if(nestegg_track_count(m->ne, &m->tracks)) MBEW_RETURN(TRACK_COUNT);

	/* Iterate through all the tracks, latching on to the first audio and video streams found. */
	for(i = 0; i < m->tracks; i++) {
		mbew_track_t* track = NULL;

		int type = nestegg_track_type(m->ne, i);
		int codec = nestegg_track_codec_id(m->ne, i);

		if(type == NESTEGG_TRACK_VIDEO) track = &m->video.track;

		else if(type == NESTEGG_TRACK_AUDIO) track = &m->audio.track;

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
			m->video.iface = codec == NESTEGG_CODEC_VP9 ?
				&vpx_codec_vp9_dx_algo :
				&vpx_codec_vp8_dx_algo
			;

			if(vpx_codec_dec_init(
				&m->video.codec,
				m->video.iface,
				NULL,
				m->flags
			)) MBEW_RETURN(INIT_CODEC);

			if(nestegg_track_video_params(
				m->ne,
				i,
				&m->video.params
			)) MBEW_RETURN(PARAMS_VIDEO);
		}

		/* The if/else if/else above will make sure we're either audio or video; never unknown. */
		else {
			if(nestegg_track_audio_params(
				m->ne,
				i,
				&m->audio.params
			)) MBEW_RETURN(PARAMS_AUDIO);
		}
	}

	return m;
}

void mbew_destroy(mbew_t m) {
	if(m->iter.active) return;

	if(m->video.track.init) vpx_codec_destroy(&m->video.codec);
	if(m->video.data.rgb) free(m->video.data.rgb);
	if(m->ne) nestegg_destroy(m->ne);

	mbew_iter_reset(m);
	mbew_src_destroy(m);

	free(m);
}

#define BOOL_RETURN(st) { m->status = MBEW_STATUS_##st; return MBEW_FALSE; }

mbew_bool_t mbew_reset(mbew_t m) {
	if(nestegg_offset_seek(m->ne, 0)) BOOL_RETURN(SEEK_OFFSET);

	if(
		m->video.track.init &&
		nestegg_track_seek(m->ne, m->video.track.index, 0)
	) BOOL_RETURN(SEEK_VIDEO);

	/* TODO: This is always failing! */
	/* if(
		m->audio.track.init &&
		nestegg_track_seek(m->ne, m->audio.track.index, 0)
	) BOOL_RETURN(SEEK_AUDIO); */

	mbew_iter_reset(m);

	return MBEW_TRUE;
}

mbew_status_t mbew_status(mbew_t m) {
	return !m ? MBEW_STATUS_NULL : m->status;
}

#define CASE_PROPERTY(prop, ty, attr) case MBEW_PROP_##prop: r.ty = m->attr; break

mbew_prop_val_t mbew_property(mbew_t m, ...) {
	mbew_prop_val_t r = { 0 };
	mbew_prop_t prop;

	va_list arg_enum;
	va_list arg_str;

	va_start(arg_enum, m);
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

static const char* MBEW_TYPE_STRINGS[] = {
	"MBEW_FALSE",
	"MBEW_TRUE",
	"MBEW_SRC_FILE",
	"MBEW_SRC_MEMORY",
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
	"MBEW_STATUS_PACKET_READ",
	"MBEW_STATUS_PACKET_TRACK",
	"MBEW_STATUS_PACKET_COUNT",
	"MBEW_STATUS_PACKET_TSTAMP",
	"MBEW_STATUS_PACKET_DURATION",
	"MBEW_STATUS_PACKET_DATA",
	"MBEW_STATUS_VPX_DECODE",
	"MBEW_STATUS_GET_FRAME",
	"MBEW_STATUS_SEEK_OFFSET",
	"MBEW_STATUS_SEEK_VIDEO",
	"MBEW_STATUS_SEEK_AUDIO",
	"MBEW_STATUS_TODO",
	"MBEW_STATUS_NOT_IMPLEMENTED",
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
	"MBEW_DATA_NONE",
	"MBEW_DATA_VIDEO",
	"MBEW_DATA_AUDIO",
	"MBEW_CODEC_VP8",
	"MBEW_CODEC_VORBIS",
	"MBEW_CODEC_VP9",
	"MBEW_CODEC_OPUS"
};

#define OFFSET_BOOL 0
#define OFFSET_SRC 2
#define OFFSET_STATUS 4
#define OFFSET_PROP 29
#define OFFSET_DATA 41
#define OFFSET_CODEC 45
#define OFFSET_MAX 48

#define VAL_BOOL 2
#define VAL_SRC 2
#define VAL_STATUS 24
#define VAL_PROP 12
#define VAL_DATA 3
#define VAL_CODEC 4

#define CASE_TYPE(ty) case MBEW_TYPE_##ty: if(val < VAL_##ty) { offset = OFFSET_##ty; } break

const char* mbew_string(mbew_type_t type, ...) {
	va_list arg;

	mbew_num_t val = 0;
	mbew_num_t offset = OFFSET_MAX;

	va_start(arg, type);

	val = va_arg(arg, mbew_num_t);

	switch(type) {
		CASE_TYPE(BOOL);
		CASE_TYPE(SRC);
		CASE_TYPE(STATUS);
		CASE_TYPE(PROP);
		CASE_TYPE(DATA);
		CASE_TYPE(CODEC);

		default:
			break;
	}

	va_end(arg);

	if(offset < OFFSET_MAX) return MBEW_TYPE_STRINGS[offset + val];

	return "ERROR";
}

