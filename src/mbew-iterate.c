#include "mbew-private.h"

#include <string.h>
#include <stdlib.h>

#define MBEW_RETURN(st) { mbew->status = MBEW_STATUS_##st; nestegg_free_packet(packet); return; }

/* TODO: Make sure we start from the beginning somehow. */
void mbew_iterate(mbew_t* mbew, mbew_iter_cb_t callback) {
	nestegg_packet* packet = NULL;
	mbew_num_t index = 0;

	unsigned char* video_buffer = NULL;

	if(mbew->video.track.init) video_buffer = (unsigned char*)(malloc(
		mbew->video.params.width *
		mbew->video.params.height *
		4
	));

	while(nestegg_read_packet(mbew->ne, &packet) > 0) {
		mbew_num_t track = 0;
		mbew_num_t count = 0;
		mbew_ns_t tstamp = 0;
		mbew_ns_t duration = 0;

		int type;

		if(nestegg_packet_track(packet, &track)) MBEW_RETURN(PACKET_TRACK);
		if(nestegg_packet_count(packet, &count)) MBEW_RETURN(PACKET_COUNT);
		if(nestegg_packet_tstamp(packet, &tstamp)) MBEW_RETURN(PACKET_TSTAMP);

		/* TODO: Why does the following code ALWAYS fail?
		if(nestegg_packet_duration(packet.ne, &packet.duration)) MBEW_RETURN(PACKET_DURATION); */

		nestegg_packet_duration(packet, &duration);

		type = nestegg_track_type(mbew->ne, track);

		if(type == NESTEGG_TRACK_VIDEO) {
			mbew_data_video_t video_data;

			unsigned char* data = NULL;
			size_t length;

			vpx_codec_stream_info_t info;
			vpx_codec_iter_t iter = NULL;
			vpx_image_t* img = NULL;

			/* TODO: Handle the case where there are multiples. */
			if(count > 1) MBEW_RETURN(TODO);

			memset(&info, 0, sizeof(vpx_codec_stream_info_t));

			info.sz = sizeof(vpx_codec_stream_info_t);

			/* Get a data pointer to the corresponding "data chunk." */
			if(nestegg_packet_data(packet, 0, &data, &length)) MBEW_RETURN(PACKET_DATA);

			vpx_codec_peek_stream_info(mbew->video.iface, data, length, &info);

			if(vpx_codec_decode(&mbew->video.codec, data, length, NULL, 0)) MBEW_RETURN(VPX_DECODE);

			/* Assume there is only one frame.
			 * TODO: Handle the case where there are multiples. */
			if((img = vpx_codec_get_frame(&mbew->video.codec, &iter))) {
				/* TODO: Handle the case where the img->d_w and img->d_h are different from the
				 * value detected in the video.params struture. */
				video_data.data = video_buffer;
				video_data.width = img->d_w;
				video_data.height = img->d_h;

				mbew_format_rgb(img, video_buffer);

				callback(MBEW_ITER_VIDEO, index, tstamp, &video_data);
			}
		}

		/* TODO: Call this when one of the above calls fails. */
		nestegg_free_packet(packet);

		index++;
	}

	if(video_buffer) free(video_buffer);
}

