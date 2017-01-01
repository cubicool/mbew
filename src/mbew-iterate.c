#include "mbew-private.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void mbew_iter_reset(mbew_t m) {
	if(m->iter.packet) nestegg_free_packet(m->iter.packet);

	memset(&m->iter, 0, sizeof(mbew_iter_t));
}

#define _return(st) { m->status = MBEW_STATUS_##st; mbew_iter_reset(m); return MBEW_FALSE; }

mbew_bool_t mbew_iterate(mbew_t m, mbew_num_t flags) {
	int e;

	/* In MBEW_ITERATE_SYNC mode, the user is expected to feed elapsed time data into the iteration
	 * state before any data is yielded. This check simply short-circuits the iteration if the
	 * pending timestamp hasn't been elapsed. */
	if(
		mbew_flags(flags, MBEW_ITERATE_SYNC) &&
		(m->iter.elapsed < m->iter.timestamp)
	) {
		m->iter.type = MBEW_DATA_SYNC;

		return MBEW_TRUE;
	}

	/* Check for any conflicting flag bits. */
	if(mbew_flags(flags, MBEW_ITERATE_VIDEO | MBEW_ITERATE_AUDIO)) _return(ITERATE_FLAGS);

	/* If iter is NULL, this is the first time iterate has been called. */
	if(!m->iter.active) {
		if(m->video.track.init && mbew_flags(flags, MBEW_ITERATE_RGB)) {
			m->video.data.rgb = (mbew_bytes_t)(malloc(
				m->video.params.width *
				m->video.params.height *
				4
			));
		}

		m->iter.active = MBEW_TRUE;
		m->iter.flags = flags;
	}

	while((e = nestegg_read_packet(m->ne, &m->iter.packet)) > 0) {
		mbew_num_t track = 0;
		mbew_num_t count = 0;
		mbew_num_t type = 0;
		mbew_ns_t duration = 0;
		mbew_bytes_t data = NULL;

		size_t size;

		if(nestegg_packet_track(m->iter.packet, &track)) _return(PACKET_TRACK);
		if(nestegg_packet_count(m->iter.packet, &count)) _return(PACKET_COUNT);
		if(nestegg_packet_tstamp(m->iter.packet, &m->iter.timestamp)) _return(PACKET_TSTAMP);

		/* TODO: Why does the following code ALWAYS fail?
		if(nestegg_packet_duration(packet.ne, &packet.duration)) _return(PACKET_DURATION); */

		/* TODO: Handle the case where there are multiples. */
		if(count > 1) _return(NOT_IMPLEMENTED);

		nestegg_packet_duration(m->iter.packet, &duration);

		type = nestegg_track_type(m->ne, track);

		if(type == NESTEGG_TRACK_VIDEO && !mbew_flags(flags, MBEW_ITERATE_AUDIO)) {
			vpx_codec_stream_info_t info;
			vpx_codec_iter_t codec_iter = NULL;
			vpx_image_t* img = NULL;

			memset(&info, 0, sizeof(vpx_codec_stream_info_t));

			info.sz = sizeof(vpx_codec_stream_info_t);

			/* Get a data pointer to the corresponding "data chunk." */
			if(nestegg_packet_data(m->iter.packet, 0, &data, &size)) _return(PACKET_DATA);

			vpx_codec_peek_stream_info(m->video.iface, data, size, &info);

			if(vpx_codec_decode(&m->video.codec, data, size, NULL, 0)) _return(VPX_DECODE);

			/* Assume there is only one frame.
			 * TODO: Handle the case where there are multiples frames-in-frame. */
			if((img = vpx_codec_get_frame(&m->video.codec, &codec_iter))) {
				/* TODO: Handle the case where the img->d_w and img->d_h are different from the
				 * value detected in the video.params struture. */
				if(mbew_flags(flags, MBEW_ITERATE_RGB)) mbew_format_rgb(img, m->video.data.rgb);

				/* Otherwise, leave the YUV420 data unmolested. */
				else {
					m->video.data.yuv.planes = img->planes;
					m->video.data.yuv.stride = (mbew_num_t*)(img->stride);
				}
			}

			else _return(GET_FRAME);

			m->iter.type = MBEW_DATA_VIDEO;

			break;
		}

		else if(type == NESTEGG_TRACK_AUDIO && !mbew_flags(flags, MBEW_ITERATE_VIDEO)) {
			if(nestegg_packet_data(m->iter.packet, 0, &data, &size)) _return(PACKET_DATA);

			/* printf("AUDIO_PACKET => size(%lu)\n", size); */

			m->iter.type = MBEW_DATA_AUDIO;

			break;
		}
	}

	/* If 0 was returned, we've reached the end of the stream. */
	if(!e) {
		mbew_iter_reset(m);

		return MBEW_FALSE;
	}

	/* Anything else indicates an error occurred. */
	else if(e < 0) _return(PACKET_READ);

	nestegg_free_packet(m->iter.packet);

	m->iter.packet = NULL;
	m->iter.index++;

	return MBEW_TRUE;
}

mbew_bool_t mbew_iter_active(mbew_t m) {
	return m->iter.active;
}

mbew_data_t mbew_iter_type(mbew_t m) {
	return m->iter.type;
}

mbew_num_t mbew_iter_index(mbew_t m) {
	return m->iter.index;
}

mbew_ns_t mbew_iter_timestamp(mbew_t m) {
	return m->iter.timestamp;
}

mbew_bool_t mbew_iter_sync(mbew_t m, mbew_ns_t elapsed) {
	if(!m->iter.active || !mbew_flags(m->iter.flags, MBEW_ITERATE_SYNC)) return MBEW_FALSE;

	m->iter.elapsed = elapsed;

	return m->iter.elapsed >= m->iter.timestamp;
}

mbew_ns_t mbew_iter_next(mbew_t m) {
	if(
		!m->iter.active ||
		!mbew_flags(m->iter.flags, MBEW_ITERATE_SYNC) ||
		(m->iter.elapsed > m->iter.timestamp)
	) return 0;

	return m->iter.timestamp - m->iter.elapsed;
}

mbew_bytes_t mbew_iter_rgb(mbew_t m) {
	return m->video.data.rgb;
}

mbew_bytes_t* mbew_iter_yuv_planes(mbew_t m) {
	return m->video.data.yuv.planes;
}

mbew_num_t* mbew_iter_yuv_stride(mbew_t m) {
	return m->video.data.yuv.stride;
}

