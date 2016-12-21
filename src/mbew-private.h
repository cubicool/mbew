#ifndef MBEW_PRIVATE_H
#define MBEW_PRIVATE_H 1

#include "mbew.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "nestegg/nestegg.h"

typedef struct _mbew_track_t {
	mbew_num_t type;
	mbew_num_t codec;
	mbew_num_t index;
	mbew_bool_t init;
} mbew_track_t;

typedef struct _mbew_audio_t {
	mbew_track_t track;

	nestegg_audio_params params;
} mbew_audio_t;

typedef struct _mbew_video_t {
	mbew_track_t track;

	nestegg_video_params params;

	vpx_codec_iface_t* iface;
	vpx_codec_ctx_t codec;
} mbew_video_t;

struct _mbew_t {
	mbew_src_t src;
	mbew_status_t status;

	/* The WebM duration in nanoseconds. */
	mbew_ns_t duration;
	mbew_ns_t scale;

	/* The number of tracks.
	 * TODO: Currently, only _TWO_ tracks (one audio and one video) are supported! */
	mbew_num_t tracks;

	/* Bitfield of the VPX_CODEC_USE_* values found in vpx_decoder.h; currently unused. */
	mbew_num_t flags;

	mbew_audio_t audio;
	mbew_video_t video;

	nestegg* ne;
	nestegg_io ne_io;
};

struct _mbew_iter_t {
	nestegg_packet* packet;

	mbew_num_t index;
	mbew_ns_t timestamp;
	mbew_data_t type;
	mbew_data_video_t video;
	mbew_data_audio_t audio;
};

mbew_bool_t mbew_create_src_file(mbew_t* mbew, void* data);
mbew_bool_t mbew_create_src_memory(mbew_t* mbew, void* data);

void mbew_format_rgb(vpx_image_t* img, uint8_t* dest);

#endif

