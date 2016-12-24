#ifndef MBEW_PRIVATE_H
#define MBEW_PRIVATE_H 1

#include "mbew.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "nestegg/nestegg.h"

#include <stdarg.h>

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

	struct {
		struct {
			mbew_bytes_t* planes;
			mbew_num_t* stride;
		} yuv;

		mbew_bytes_t rgb;
	} data;
} mbew_video_t;

typedef struct _mbew_iter_t {
	nestegg_packet* packet;

	mbew_bool_t active;
	mbew_data_t type;
	mbew_num_t flags;
	mbew_num_t index;
	mbew_ns_t timestamp;
	mbew_ns_t elapsed;
} mbew_iter_t;

struct _mbew_t {
	nestegg* ne;
	nestegg_io ne_io;

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

	/* Used heavily during mbew_iterate(). */
	mbew_iter_t iter;
};

mbew_bool_t mbew_src_create(mbew_src_t src, mbew_t m, va_list args);
void mbew_src_destroy(mbew_t m);

void mbew_format_rgb(vpx_image_t* img, uint8_t* dest);

void mbew_iter_reset(mbew_t m);

#endif

