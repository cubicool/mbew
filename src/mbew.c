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

#define _return(st) { m->status = MBEW_STATUS_##st; return m; }

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
		if(nestegg_init(&m->ne, m->ne_io, mbew_log, -1)) _return(INIT_IO);
		if(nestegg_duration(m->ne, &m->duration)) _return(DURATION);
		if(nestegg_tstamp_scale(m->ne, &m->scale)) _return(SCALE);
		if(nestegg_track_count(m->ne, &m->tracks)) _return(TRACK_COUNT);

		/* Iterate through all the tracks, latching on to the first audio and video streams
		 * found. */
		for(t = 0; t < m->tracks; t++) {
			mbew_track_t* track = NULL;

			mbew_num_t type = nestegg_track_type(m->ne, t);
			mbew_num_t codec = nestegg_track_codec_id(m->ne, t);

			if(type == NESTEGG_TRACK_VIDEO) track = &m->video.track;

			else if(type == NESTEGG_TRACK_AUDIO) track = &m->audio.track;

			else _return(UNKNOWN_TRACK);

			/* If a previous audio/video track was found, ignore the current one.
			 * TODO: Expand the API so that it can handle multiple tracks of the same type; I
			 * couldn't find a sample WebM video with more than 2 tracks, however. */
			if(track->init) continue;

			track->type = type;
			track->index = t;
			track->codec = mbew_enum(CODEC, codec);

			if(type == NESTEGG_TRACK_VIDEO) {
				if(nestegg_track_video_params(m->ne, t, &m->video.params)) _return(PARAMS_VIDEO);

				m->video.iface = codec == NESTEGG_CODEC_VP9 ?
					&vpx_codec_vp9_dx_algo :
					&vpx_codec_vp8_dx_algo
				;

				if(vpx_codec_dec_init(
					&m->video.codec,
					m->video.iface,
					NULL,
					m->flags
				)) _return(INIT_CODEC);
			}

			/* The if/else if/else above will make sure we're either audio or video; never
			 * unknown. */
			else {
				mbew_num_t headers = 0;
				mbew_num_t h;

				/* We only support VORBIS for now. */
				if(track->codec != MBEW_CODEC_VORBIS) _return(NOT_IMPLEMENTED);

				if(nestegg_track_audio_params(m->ne, t, &m->audio.params)) _return(PARAMS_AUDIO);
				if(nestegg_track_codec_data_count(m->ne, t, &headers)) _return(CODEC_DATA_COUNT);

				/* TODO: Vorbis data should always have 3 headers! Check this more accurately. */
				if(headers != 3) _return(CODEC_DATA_COUNT);

				/* for(h = 0; h < headers; h++) {
					if(nestegg_track_codec_data(
						m->ne,
						t,
						h,
						&m->audio.vorbis.headers[h].data,
						&m->audio.vorbis.headers[h].size
					)) _return(CODEC_DATA);
				}

				vorbis_info_init(&m->audio.vorbis.info);
				vorbis_comment_init(&m->audio.vorbis.comment);

				for(h = 0; h < headers; h++) {
					vorbis_synthesis_headerin(
						&m->audio.vorbis.info,
						&m->audio.vorbis.comment,
						(ogg_packet*)(&m->audio.vorbis.headers[0].data)
					);
				}

				vorbis_comment_clear(&m->audio.vorbis.comment);
				vorbis_synthesis_init(&m->audio.vorbis.dsp_state, &m->audio.vorbis.info);
				vorbis_block_init(&m->audio.vorbis.dsp_state, &m->audio.vorbis.block); */
			}

			/* If we make it this far without erroring, assume we are initialized successfully. */
			track->init = MBEW_TRUE;
		}

		m->status = MBEW_STATUS_VALID;
	}

	return m;
}

void mbew_destroy(mbew_t m) {
	if(m->iter.active) return;

	if(m->video.track.init) {
		vpx_codec_destroy(&m->video.codec);

		if(m->video.data.rgb) free(m->video.data.rgb);
	}

	/* if(m->audio.track.init) {
		vorbis_block_clear(&m->audio.vorbis.block);
		vorbis_dsp_clear(&m->audio.vorbis.dsp_state);
		vorbis_info_clear(&m->audio.vorbis.info);
	} */

	if(m->ne) nestegg_destroy(m->ne);

	mbew_iter_reset(m);
	mbew_src_destroy(m);

	free(m);
}

#undef _return
#define _return(st) { m->status = MBEW_STATUS_##st; return MBEW_FALSE; }

mbew_bool_t mbew_reset(mbew_t m) {
	if(nestegg_offset_seek(m->ne, 0)) _return(SEEK_OFFSET);

	if(
		m->video.track.init &&
		nestegg_track_seek(m->ne, m->video.track.index, 0)
	) _return(SEEK_VIDEO);

	/* TODO: This is always failing! */
	/* if(
		m->audio.track.init &&
		nestegg_track_seek(m->ne, m->audio.track.index, 0)
	) _return(SEEK_AUDIO); */

	mbew_iter_reset(m);

	return MBEW_TRUE;
}

mbew_status_t mbew_status(mbew_t m) {
	return !m ? MBEW_STATUS_NULL_CONTEXT : m->status;
}

#define _case(prop, ty, attr) case MBEW_PROPERTY_##prop: r.ty = m->attr; break

mbew_propval_t mbew_property(mbew_t m, ...) {
	mbew_propval_t r = { 0 };
	mbew_property_t prop;

	va_list arg_enum;
	va_list arg_str;

	va_start(arg_enum, m);
	va_copy(arg_str, arg_enum);

	prop = va_arg(arg_enum, mbew_property_t);

	/* TODO: Handle string value. */
	/* if(prop >= MBEW_PROP_MAX) {
		const char* str = va_arg(arg_str, const char*);
	} */

	va_end(arg_str);
	va_end(arg_enum);

	switch(prop) {
		_case(DURATION, ns, duration);
		_case(SCALE, ns, scale);
		_case(TRACKS, num, tracks);
		_case(VIDEO, b, video.track.init);
		_case(VIDEO_TRACK, num, video.track.index);
		_case(VIDEO_CODEC, num, video.track.codec);
		_case(VIDEO_WIDTH, num, video.params.width);
		_case(VIDEO_HEIGHT, num, video.params.height);
		_case(AUDIO, b, audio.track.init);
		_case(AUDIO_TRACK, num, audio.track.index);
		_case(AUDIO_CODEC, num, audio.track.codec);
		_case(AUDIO_RATE, hz, audio.params.rate);
		_case(AUDIO_CHANNELS, num, audio.params.channels);
		_case(AUDIO_DEPTH, num, audio.params.depth);

		default:
			/* TODO: Set error status. */
			break;
	}

	return r;
}

