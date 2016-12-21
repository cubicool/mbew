#ifndef MBEW_H
#define MBEW_H 1

#define MBEW_API_BEGIN
#define MBEW_API_END
#define MBEW_API

#if defined(MBEW_API_PUBLIC)
	#ifdef  __cplusplus
		#define MBEW_API_BEGIN extern "C" {
		#define MBEW_API_END }
	#endif

	#if defined(_MSC_VER)
		#define MBEW_API __declspec(dllimport)
	#endif
#endif

#include <limits.h>
#include <stdint.h>

MBEW_API_BEGIN

typedef struct _mbew_t mbew_t;

typedef enum _mbew_src_t {
	/* Reads a WebM file from disk using the traditional fopen/fclose/etc. interface. */
	MBEW_SRC_FILE,

	/* Reads a WebM file from a buffer of memory. */
	MBEW_SRC_MEMORY
} mbew_src_t;

/* Creates a new context for the given source type. */
MBEW_API mbew_t* mbew_create(mbew_src_t src, void* data);

/* Destroys a previously created context. */
MBEW_API void mbew_destroy(mbew_t* mbew);

typedef enum _mbew_status_t {
	MBEW_STATUS_SUCCESS = 0,
	MBEW_STATUS_NULL = 1,
	MBEW_STATUS_SRC_FILE = 2,
	MBEW_STATUS_SRC_MEMORY = 3,
	MBEW_STATUS_INIT_IO = 4,
	MBEW_STATUS_INIT_CODEC = 5,
	MBEW_STATUS_DURATION = 6,
	MBEW_STATUS_TRACK_COUNT = 7,
	MBEW_STATUS_UNKNOWN_TRACK = 8,
	MBEW_STATUS_PARAMS_VIDEO = 9,
	MBEW_STATUS_PARAMS_AUDIO = 10,
	MBEW_STATUS_PACKET_TRACK = 11,
	MBEW_STATUS_PACKET_COUNT = 12,
	MBEW_STATUS_PACKET_TSTAMP = 13,
	MBEW_STATUS_PACKET_DURATION = 14,
	MBEW_STATUS_PACKET_DATA = 15,
	MBEW_STATUS_VPX_DECODE = 16,
	MBEW_STATUS_TODO = 17,
	MBEW_STATUS_NOT_IMPLEMENTED = 18
} mbew_status_t;

/* Returns the most recent status associated with the given context. */
MBEW_API mbew_status_t mbew_status(mbew_t* mbew);

typedef uint64_t mbew_ns_t;
typedef unsigned int mbew_num_t;
typedef double mbew_hz_t;

typedef enum _mbew_bool_t {
	MBEW_FALSE = 0,
	MBEW_TRUE
} mbew_bool_t;

typedef enum _mbew_prop_t {
	/* The stream duration in nanoseconds.
	 *
	 * [key: duration] [val: mbew_ns_t] */
	MBEW_PROP_DURATION = 1,

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
	MBEW_PROP_AUDIO_DEPTH,

	MBEW_PROP_MAX
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

#if 0
typedef enum _mbew_type_t {
	/* Enumerations... */
	MBEW_TYPE_SRC,
	MBEW_TYPE_STATUS,
	MBEW_TYPE_BOOL,
	MBEW_TYPE_PROP,
	MBEW_TYPE_TYPE,

	/* Structs/Unions.... */
	MBEW_TYPE_MBEW,
	MBEW_TYPE_PROP_VAL
} mbew_type_t;

MBEW_API const char* mbew_type(mbew_type_t type, ...);
#endif

typedef enum _mbew_iter_t {
	MBEW_ITER_VIDEO,
	MBEW_ITER_AUDIO
} mbew_iter_t;

typedef void (*mbew_iter_cb_t)(
	mbew_iter_t type,
	mbew_num_t index,
	mbew_ns_t tstamp,
	const void* data
);

typedef struct _mbew_data_video_t {
	const void* data;
	mbew_num_t width;
	mbew_num_t height;
} mbew_data_video_t;

typedef struct _mbew_data_audio_t {
	const void* data;
} mbew_data_audio_t;

MBEW_API void mbew_iterate(mbew_t* mbew, mbew_iter_cb_t cb);

MBEW_API_END

#endif

