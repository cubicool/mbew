#include "mbew-private.h"

#include <string.h>
#include <stdlib.h>

#define MBEW_RETURN(st) { \
	mbew->status = MBEW_STATUS_##st; \
	nestegg_free_packet(iter->packet); \
	mbew_iter_destroy(iter); \
	return NULL; \
}

mbew_iter_t* mbew_iterate(mbew_t* mbew, mbew_iter_t* iter) {
	/* If iter is NULL, this is the first time iterate has been called. */
	if(!iter) {
		iter = (mbew_iter_t*)(calloc(1, sizeof(mbew_iter_t)));

		if(mbew->video.track.init) {
			iter->video.data = (unsigned char*)(malloc(
				mbew->video.params.width *
				mbew->video.params.height *
				4
			));

			iter->video.width = mbew->video.params.width;
			iter->video.height = mbew->video.params.height;
		}
	}

	if(nestegg_read_packet(mbew->ne, &iter->packet) > 0) {
		mbew_num_t track = 0;
		mbew_num_t count = 0;
		mbew_ns_t duration = 0;

		int type;

		if(nestegg_packet_track(iter->packet, &track)) MBEW_RETURN(PACKET_TRACK);
		if(nestegg_packet_count(iter->packet, &count)) MBEW_RETURN(PACKET_COUNT);
		if(nestegg_packet_tstamp(iter->packet, &iter->timestamp)) MBEW_RETURN(PACKET_TSTAMP);

		/* TODO: Why does the following code ALWAYS fail?
		if(nestegg_packet_duration(packet.ne, &packet.duration)) MBEW_RETURN(PACKET_DURATION); */

		nestegg_packet_duration(iter->packet, &duration);

		type = nestegg_track_type(mbew->ne, track);

		if(type == NESTEGG_TRACK_VIDEO) {
			unsigned char* data = NULL;
			size_t length;

			vpx_codec_stream_info_t info;
			vpx_codec_iter_t codec_iter = NULL;
			vpx_image_t* img = NULL;

			/* TODO: Handle the case where there are multiples. */
			if(count > 1) MBEW_RETURN(TODO);

			memset(&info, 0, sizeof(vpx_codec_stream_info_t));

			info.sz = sizeof(vpx_codec_stream_info_t);

			/* Get a data pointer to the corresponding "data chunk." */
			if(nestegg_packet_data(iter->packet, 0, &data, &length)) MBEW_RETURN(PACKET_DATA);

			vpx_codec_peek_stream_info(mbew->video.iface, data, length, &info);

			if(vpx_codec_decode(&mbew->video.codec, data, length, NULL, 0)) MBEW_RETURN(VPX_DECODE);

			/* Assume there is only one frame.
			 * TODO: Handle the case where there are multiples. */
			if((img = vpx_codec_get_frame(&mbew->video.codec, &codec_iter))) {
				/* TODO: Handle the case where the img->d_w and img->d_h are different from the
				 * value detected in the video.params struture. */
				mbew_format_rgb(img, iter->video.data);
			}
		}

		/* TODO: Call this when one of the above calls fails. */
		nestegg_free_packet(iter->packet);

		iter->index++;
	}

	/* Otherwise, we've reached the end. */
	else {
		mbew_iter_destroy(iter);

		iter = NULL;
	}

	return iter;
}

mbew_num_t mbew_iter_index(mbew_iter_t* iter) {
	return iter->index;
}

mbew_ns_t mbew_iter_timestamp(mbew_iter_t* iter) {
	return iter->timestamp;
}

mbew_data_t mbew_iter_type(mbew_iter_t* iter) {
	return iter->type;
}

mbew_data_video_t* mbew_iter_video(mbew_iter_t* iter) {
	return &iter->video;
}

mbew_data_audio_t* mbew_iter_audio(mbew_iter_t* iter) {
	return &iter->audio;
}

void mbew_iter_destroy(mbew_iter_t* iter) {
	if(iter->video.data) free(iter->video.data);

	free(iter);
}

