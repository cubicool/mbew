#ifndef MBEW_H
#define MBEW_H 1

#ifdef  __cplusplus
	#define MBEW_API_BEGIN extern "C" {
	#define MBEW_API_END }
#else
	#define MBEW_API_BEGIN
	#define MBEW_API_END
#endif

#if defined(_MSC_VER)
	#define MBEW_API __declspec(dllimport)
#else
	#define MBEW_API
#endif

#include <limits.h>
#include <stdint.h>

MBEW_API_BEGIN

typedef struct _mbew_t mbew_t;

typedef enum _mbew_bool_t {
	MBEW_FALSE,
	MBEW_TRUE
} mbew_bool_t;

typedef uint64_t mbew_ns_t;
typedef unsigned int mbew_num_t;
typedef double mbew_hz_t;
typedef unsigned char* mbew_bytes_t;

typedef enum _mbew_src_t {
	/* Reads a WebM file from disk using the traditional fopen/fclose/etc. interface. A single
	 * argument is passed to mbew_create(), which is the path to file to be opened. */
	MBEW_SRC_FILE,

	/* Reads a WebM file from a buffer of memory. Two arguments are passed to mbew_create(): a
	 * void* pointer to the memory location and a size_t argument indicating its size. */
	MBEW_SRC_MEMORY
} mbew_src_t;

/* Creates a new context for the given source type. */
MBEW_API mbew_t* mbew_create(mbew_src_t src, ...);

/* Destroys a previously created context. */
MBEW_API void mbew_destroy(mbew_t* mbew);

MBEW_API mbew_bool_t mbew_reset(mbew_t* mbew);

typedef enum _mbew_status_t {
	MBEW_STATUS_SUCCESS,
	MBEW_STATUS_NULL,
	MBEW_STATUS_SRC_FILE,
	MBEW_STATUS_SRC_MEMORY,
	MBEW_STATUS_INIT_IO,
	MBEW_STATUS_INIT_CODEC,
	MBEW_STATUS_DURATION,
	MBEW_STATUS_SCALE,
	MBEW_STATUS_TRACK_COUNT,
	MBEW_STATUS_UNKNOWN_TRACK,
	MBEW_STATUS_PARAMS_VIDEO,
	MBEW_STATUS_PARAMS_AUDIO,
	MBEW_STATUS_PACKET_READ,
	MBEW_STATUS_PACKET_TRACK,
	MBEW_STATUS_PACKET_COUNT,
	MBEW_STATUS_PACKET_TSTAMP,
	MBEW_STATUS_PACKET_DURATION,
	MBEW_STATUS_PACKET_DATA,
	MBEW_STATUS_VPX_DECODE,
	MBEW_STATUS_GET_FRAME,
	MBEW_STATUS_SEEK_OFFSET,
	MBEW_STATUS_SEEK_VIDEO,
	MBEW_STATUS_SEEK_AUDIO,
	MBEW_STATUS_ITER_BUSY,
	MBEW_STATUS_TODO,
	MBEW_STATUS_NOT_IMPLEMENTED
} mbew_status_t;

/* Returns the most recent status associated with the given context. */
MBEW_API mbew_status_t mbew_status(mbew_t* mbew);

typedef enum _mbew_prop_t {
	/* The stream duration in nanoseconds.
	 *
	 * [key: duration] [val: mbew_ns_t] */
	MBEW_PROP_DURATION,

	/* The scale for each timestamp value.
	 *
	 * [key: scale] [val: mbew_ns_t] */
	MBEW_PROP_SCALE,

	/* The total number of tracks.
	 *
	 * [key: tracks] [val: mbew_num_t] */
	MBEW_PROP_TRACKS,

	/* True if a video track was found, false otherwise.
	 *
	 * [key: video] [val: mbew_bool_t] */
	MBEW_PROP_VIDEO,

	/* The numeric index of the video track.
	 *
	 * [key: video.track] [val: mbew_num_t] */
	MBEW_PROP_VIDEO_TRACK,

	/* Pixel-width of the video.
	 *
	 * [key: video.width] [val: mbew_num_t] */
	MBEW_PROP_VIDEO_WIDTH,

	/* Pixel-height of the video.
	 *
	 * [key: video.width] [val: mbew_num_t] */
	MBEW_PROP_VIDEO_HEIGHT,

	/* True if a audio track was found, false otherwise.
	 *
	 * [key: audio] [val: mbew_bool_t] */
	MBEW_PROP_AUDIO,

	/* The numeric index of the audio track.
	 *
	 * [key: audio.track] [val: mbew_num_t] */
	MBEW_PROP_AUDIO_TRACK,

	/* Sampling rate in HZ.
	 *
	 * [key: audio.rate] [val: mbew_hz_t] */
	MBEW_PROP_AUDIO_RATE,

	/* Number of audio channels.
	 *
	 * [key: audio.channels] [val: mbew_num_t] */
	MBEW_PROP_AUDIO_CHANNELS,

	/* Bits per sample.
	 *
	 * [key: audio.depth] [val: mbew_num_t] */
	MBEW_PROP_AUDIO_DEPTH
} mbew_prop_t;

/* This union encapsulates each kind of single value that can be returned as property. */
typedef union _mbew_prop_val_t {
	mbew_ns_t ns;
	mbew_num_t num;
	mbew_hz_t hz;
	mbew_bool_t b;
} mbew_prop_val_t;

/* Accepts either a single mbew_prop_t value OR string, returning the resultant value
 * encapsulated within a mbew_prop_val_t union. */
MBEW_API mbew_prop_val_t mbew_property(mbew_t* mbew, ...);

MBEW_API void mbew_properties(mbew_t* mbew, ...);

typedef enum _mbew_type_t {
	MBEW_TYPE_BOOL,
	MBEW_TYPE_SRC,
	MBEW_TYPE_STATUS,
	MBEW_TYPE_PROP,
	MBEW_TYPE_DATA,
	MBEW_TYPE_ITER_BIT
} mbew_type_t;

MBEW_API const char* mbew_string(mbew_type_t type, ...);

typedef enum _mbew_data_t {
	MBEW_DATA_NONE,
	MBEW_DATA_VIDEO,
	MBEW_DATA_AUDIO
} mbew_data_t;

typedef enum _mbew_iter_bit_t {
	MBEW_ITER_VIDEO_ONLY = (1 << 0),
	MBEW_ITER_AUDIO_ONLY = (1 << 1),
	MBEW_ITER_SYNC = (1 << 2),
	MBEW_ITER_FORMAT_RGB = (1 << 3)
} mbew_iter_bit_t;

MBEW_API mbew_bool_t mbew_iterate(mbew_t* mbew, mbew_num_t flags);

MBEW_API mbew_bool_t mbew_iter_active(mbew_t* mbew);
MBEW_API mbew_data_t mbew_iter_type(mbew_t* mbew);
MBEW_API mbew_num_t mbew_iter_index(mbew_t* mbew);
MBEW_API mbew_ns_t mbew_iter_timestamp(mbew_t* mbew);
MBEW_API mbew_bool_t mbew_iter_sync(mbew_t* mbew, mbew_ns_t elapsed);
MBEW_API mbew_bytes_t mbew_iter_video_rgb(mbew_t* mbew);
MBEW_API mbew_bytes_t* mbew_iter_video_yuv_planes(mbew_t* mbew);
MBEW_API mbew_num_t* mbew_iter_video_yuv_stride(mbew_t* mbew);

MBEW_API_END

#endif

