#include "mbew-private.h"

#include "vpx/vp8dx.h"

#include <stdlib.h>

static void mbew_log(nestegg* ne, mbew_num_t severity, const char* format, ...) {
#if 0
	va_list args;

	va_start(args, format);

	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");

	va_end(args);
#endif
}

mbew_t mbew_create(mbew_source_t src, ...) {
	mbew_t m = (mbew_t)(calloc(1, sizeof(struct _mbew_t)));

	va_list args;

	/* If calloc() fails for some reason, return NULL. */
	if(!m) return NULL;

	va_start(args, src);

	/* mbew_src_create() will properly set the error status, if any. */
	if(mbew_src_create(src, m, args)) {
		mbew_num_t t;

		va_end(args);

		/* Regardless of the src, begin calling the "common" setup routines. */
		if(nestegg_init(&m->ne, m->ne_io, mbew_log, -1)) mbew_fail(NESTEGG_INIT);
		if(nestegg_duration(m->ne, &m->duration)) mbew_fail(NESTEGG_DURATION);
		if(nestegg_tstamp_scale(m->ne, &m->scale)) mbew_fail(NESTEGG_TSTAMP_SCALE);
		if(nestegg_track_count(m->ne, &m->tracks)) mbew_fail(NESTEGG_TRACK_COUNT);

		/* Iterate through all the tracks, latching on to the first audio and video streams
		 * found. */
		for(t = 0; t < m->tracks; t++) {
			mbew_track_t* track = NULL;

			mbew_num_t type = nestegg_track_type(m->ne, t);
			mbew_num_t codec = nestegg_track_codec_id(m->ne, t);

			if(type == NESTEGG_TRACK_VIDEO) track = &m->video.track;

			else if(type == NESTEGG_TRACK_AUDIO) track = &m->audio.track;

			else mbew_fail(NESTEGG_TRACK_TYPE);

			/* If a previous audio/video track was found, ignore the current one.
			 * TODO: Expand the API so that it can handle multiple tracks of the same type; I
			 * couldn't find a sample WebM video with more than 2 tracks, however. */
			if(track->init) continue;

			track->type = type;
			track->index = t;
			track->codec = mbew_enum(CODEC, codec);

			if(type == NESTEGG_TRACK_VIDEO) {
				if(nestegg_track_video_params(
					m->ne,
					t,
					&m->video.params
				)) mbew_fail(NESTEGG_TRACK_VIDEO_PARAMS);

				m->video.iface = codec == NESTEGG_CODEC_VP9 ?
					&vpx_codec_vp9_dx_algo :
					&vpx_codec_vp8_dx_algo
				;

				if(vpx_codec_dec_init(
					&m->video.codec,
					m->video.iface,
					NULL,
					m->flags
				)) mbew_fail(VPX_CODEC_DEC_INIT);
			}

			/* The if/else if/else above will make sure we're either audio or video; never
			 * unknown. */
			else {
				mbew_num_t headers = 0;
				mbew_num_t h;

				/* We only support VORBIS for now. */
				if(track->codec != MBEW_CODEC_VORBIS) mbew_fail(NOT_IMPLEMENTED);

				if(nestegg_track_audio_params(
					m->ne,
					t,
					&m->audio.params
				)) mbew_fail(NESTEGG_TRACK_AUDIO_PARAMS);

				if(nestegg_track_codec_data_count(
					m->ne,
					t,
					&headers
				)) mbew_fail(NESTEGG_TRACK_CODEC_DATA_COUNT);

				/* TODO: Vorbis data should always have 3 headers! Check this more accurately. */
				if(headers != 3) mbew_fail(VORBIS_HEADER_COUNT);

				for(h = 0; h < headers; h++) {
					if(nestegg_track_codec_data(
						m->ne,
						t,
						h,
						&m->audio.vorbis.headers[h].data,
						&m->audio.vorbis.headers[h].size
					)) mbew_fail(NESTEGG_TRACK_CODEC_DATA);
				}

				vorbis_info_init(&m->audio.vorbis.info);
				vorbis_comment_init(&m->audio.vorbis.comment);

				for(h = 0; h < headers; h++) {
					if(vorbis_synthesis_headerin(
						&m->audio.vorbis.info,
						&m->audio.vorbis.comment,
						(ogg_packet*)(&m->audio.vorbis.headers[h].data)
					)) mbew_fail(VORBIS_SYNTHESIS_HEADERIN);
				}

				vorbis_comment_clear(&m->audio.vorbis.comment);

				if(vorbis_synthesis_init(
					&m->audio.vorbis.dsp,
					&m->audio.vorbis.info
				)) mbew_fail(VORBIS_SYNTHESIS_INIT);

				if(vorbis_block_init(
					&m->audio.vorbis.dsp,
					&m->audio.vorbis.block
				)) mbew_fail(VORBIS_BLOCK_INIT);
			}

			/* If we make it this far without erroring, assume we are initialized successfully. */
			track->init = MBEW_TRUE;
		}

		m->status = MBEW_STATUS_VALID;
	}

fail:
	return m;
}

void mbew_destroy(mbew_t m) {
	if(m->iter.active) return;

	if(m->video.track.init) {
		vpx_codec_destroy(&m->video.codec);

		if(m->video.data.rgb) free(m->video.data.rgb);
	}

	if(m->audio.track.init) {
		vorbis_block_clear(&m->audio.vorbis.block);
		vorbis_dsp_clear(&m->audio.vorbis.dsp);
		vorbis_info_clear(&m->audio.vorbis.info);
	}

	if(m->ne) nestegg_destroy(m->ne);

	mbew_iter_reset(m);
	mbew_src_destroy(m);

	free(m);
}

mbew_bool_t mbew_reset(mbew_t m) {
	if(nestegg_offset_seek(m->ne, 0)) mbew_fail(NESTEGG_OFFSET_SEEK);

	if(
		m->video.track.init &&
		nestegg_track_seek(m->ne, m->video.track.index, 0)
	) mbew_fail(NESTEGG_TRACK_SEEK);

	/* TODO: Why does this never succeed? */
	/* if(
		m->audio.track.init &&
		nestegg_track_seek(m->ne, m->audio.track.index, 0)
	) mbew_fail(SEEK_AUDIO); */

	mbew_iter_reset(m);

	return MBEW_TRUE;

fail:
	return MBEW_FALSE;
}

mbew_status_t mbew_status(mbew_t m) {
	return !m ? MBEW_STATUS_NULL_CONTEXT : m->status;
}

mbew_propval_t mbew_property(mbew_t m, ...) {
	mbew_propval_t r = { 0 };
	mbew_property_t prop;

	va_list arg_enum;
	va_list arg_str;

	va_start(arg_enum, m);
	va_copy(arg_str, arg_enum);

	prop = va_arg(arg_enum, mbew_property_t);

	/* TODO: Handle string value. */
	/* const char* str = va_arg(arg_str, const char*); */

	va_end(arg_str);
	va_end(arg_enum);

	switch(prop) {
		case MBEW_PROPERTY_DURATION: r.ns = m->duration; break;
		case MBEW_PROPERTY_SCALE: r.ns = m->scale; break;
		case MBEW_PROPERTY_TRACKS: r.num = m->tracks; break;
		case MBEW_PROPERTY_VIDEO: r.b = m->video.track.init; break;
		case MBEW_PROPERTY_VIDEO_TRACK: r.num = m->video.track.index; break;
		case MBEW_PROPERTY_VIDEO_CODEC: r.num = m->video.track.codec; break;
		case MBEW_PROPERTY_VIDEO_WIDTH: r.num = m->video.params.width; break;
		case MBEW_PROPERTY_VIDEO_HEIGHT: r.num = m->video.params.height; break;
		case MBEW_PROPERTY_AUDIO: r.b = m->audio.track.init; break;
		case MBEW_PROPERTY_AUDIO_TRACK: r.num = m->audio.track.index; break;
		case MBEW_PROPERTY_AUDIO_CODEC: r.num = m->audio.track.codec; break;
		case MBEW_PROPERTY_AUDIO_RATE: r.hz = m->audio.params.rate; break;
		case MBEW_PROPERTY_AUDIO_CHANNELS: r.num = m->audio.params.channels; break;
		case MBEW_PROPERTY_AUDIO_DEPTH: r.num = m->audio.params.depth; break;

		default:
			/* TODO: Set error status. */
			break;
	}

	return r;
}

