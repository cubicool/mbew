#ifndef MBEW_H
#define MBEW_H 1

#include <stdint.h>

#if defined(_MSC_VER)
	#define MBEW_API __declspec(dllimport)
#else
	#define MBEW_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* The private #mbew_t structure is the main context object within mbew, managing all the necessary
 * state for a single WebM source. To minimize dependencies--and to make libmbew seem as sleek and
 * sexy as possible--all of the implementation details are abstracted away within the sources. */
typedef struct _mbew_t* mbew_t;

/* Standard FALSE(zero) and TRUE (non-zero) values, usually preset somewhere in your include
 * hierarchy as simply TRUE and FALSE. */
typedef enum _mbew_bool_t {
	MBEW_FALSE,
	MBEW_TRUE
} mbew_bool_t;

/* A special type for nanoseconds, as they are commonly used. */
typedef uint64_t mbew_ns_t;

/* Rather than use all manner of disjoint numeric types, mbew settles on a single, simple type. */
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

/* Creates a new #mbew_t context for the given source type. Depending on the #mbew_src_t enumeration
 * specified, one ore more variable arguments will be necessary; see the #mbew_src_t documentation
 * for more info.
 *
 * To determine whether or not the context was created successfully, use the mbew_status() function,
 * as simply relying on the #mbew_t context itself to be non-NULL is not sufficient. */
MBEW_API mbew_t mbew_create(mbew_src_t src, ...);

/* Destroys a previously created context. An #mbew_t context that is currently being iterated over
 * (which you determine using mbew_iter_active()) cannot be destroyed until it either finishes it
 * iteration and or is manually reset via mbew_reset(). */
MBEW_API void mbew_destroy(mbew_t m);

/* Resets all audio, video, and iteration state back to their defaults. Returns MBEW_TRUE if all of
 * the reset operations completed successfully, MBEW_FALSE otherwise. Additional details on failure
 * can be queried using mbew_status(). */
MBEW_API mbew_bool_t mbew_reset(mbew_t m);

/* An #mbew_t context maintains an internal error tracking state that is reported to userspace via
 * the mbew_status() function. The #mbew_status_t enumerations represent to the various ways in
 * which things can sometimes fail internally. */
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
	MBEW_STATUS_ITER_FLAGS,
	MBEW_STATUS_NOT_IMPLEMENTED
} mbew_status_t;

/* Returns the most recent status associated with the given context. */
MBEW_API mbew_status_t mbew_status(mbew_t m);

/* Properties represent the high-level attributes of a WebmM source. Each property can be accessed
 * using the #mbew_prop_t enum directly OR by using the corresponding string name.
 *
 * NOTE: Take special care that you access the proper union member when fetching a property! */
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

/* This union encapsulates each kind of single value that can be returned as property. The
 * #mbew_prop_t enums above each indicate (using the [val: mbew_num_t] syntax) what low-level type
 * is used to hold its data, and userspace could should make sure to access the appropriate member
 * for the given property. */
typedef union _mbew_prop_val_t {
	mbew_ns_t ns;
	mbew_num_t num;
	mbew_hz_t hz;
	mbew_bool_t b;
} mbew_prop_val_t;

/* Accepts either a single #mbew_prop_t value OR string, returning the resultant value
 * encapsulated within a #mbew_prop_val_t union. */
MBEW_API mbew_prop_val_t mbew_property(mbew_t m, ...);

/* The #mbew_data_t enum is used to signify to client code what "type" of data is currently being
 * yielded during iteration. The MBEW_DATA_NONE value is only encountered when the MBEW_ITER_SYNC
 * #mbew_iter_bit_t is set, and is set when the caller is not synchronized with the iteration. */
typedef enum _mbew_data_t {
	MBEW_DATA_NONE,
	MBEW_DATA_VIDEO,
	MBEW_DATA_AUDIO
} mbew_data_t;

/* WebM currently only supports the following 4 codecs: 2 video, 2 audio. */
typedef enum _mbew_codec_t {
	MBEW_CODEC_VP8,
	MBEW_CODEC_VORBIS,
	MBEW_CODEC_VP9,
	MBEW_CODEC_OPUS
} mbew_codec_t;

/* The #mbew_iter_bit_t values are used to control the behavior of the mbew_iterate() function. Some
 * of the values are mutually-exclusive; see the specific comments for additional details. */
typedef enum _mbew_iter_bit_t {
	/* Only MBEW_DATA_VIDEO (and MBEW_DATA_NONE if necessary) data is exposed to the caller during
	 * the stream iteration. */
	MBEW_ITER_VIDEO_ONLY = (1 << 0),

	/* This value works on audio data just as MBEW_ITER_VIDEO_ONLY does for video. The two fields
	 * CANNOT be used in conjuction; doing so will cause mbew_iterate() to error. */
	MBEW_ITER_AUDIO_ONLY = (1 << 1),

	/* When this flag is specified, mbew_iterate()'s behavior is expanded to begin using time data,
	 * provided by the caller via the mbew_iter_sync() function, to help determine whether it should
	 * advance to the next available frame for processing or simply yield a kind of "no-op" state,
	 * essentially informing the caller that they should "try again later." This provides the caller
	 * additional flexibility in situations where blocking code is undesired, but where
	 * mbew_iterate() will likely be called MUCH faster than the sources actual framerate.
	 *
	 * When MBEW_ITER_SYNC is enabled, the #mbew_t iteration state relies on the caller to provide
	 * time information using the mbew_iter_sync() function. Once the "elapsed" time equals or
	 * exceeds the timestamp of the current iterations queued up frame, mbew_iter_sync() will return
	 * MBEW_TRUE, informing the caller that the next frame is currently bound to the current
	 * iteration and can be accessed safely. */
	MBEW_ITER_SYNC = (1 << 2),

	/* By default, video data is accessed using the mbew_iter_video_yuv() functions (which prevents
	 * libmbew from having to do potentially costly per-frame format conversion). However, if the
	 * caller needs access to standard RGB data instead, using this flag will instruct the
	 * interation process to composite all of the YUV420 data into a typical 4-byte RGBA (with alpha
	 * being unused) color buffer. This data is accessed via mbew_iter_video_rgb(). */
	MBEW_ITER_FORMAT_RGB = (1 << 3)
} mbew_iter_bit_t;

/* The mbew_iterate() routine is the primary workhorse function within libmbew, whose main purpose
 * is to loop through all of the available stream data and expose it to the caller, frame-by-frame,
 * using the mbew_iter() API described below.
 *
 * When a caller begins the iteration process, the #mbew_t context can be interacted with using the
 * mbew_iter_*() family of functions. The routines only work properly DURING the iteration process
 * facilitated by the mbew_iterate(); libmbew refers to thie as being "active", and this state can
 * be queried at any time using mbew_iter_active() (unlike most iteration functions).
 *
 * For more quick examples of the iteration API in action, use the following link:
 *
 *		http://github.com/cubicool/mbew
 *
 * NOTE: An #mbew_t context can only manage a single iteration per stream; it is NOT (currently)
 * safe to call mbew_iterate() simultaneously using the same #mbew_t context. */
MBEW_API mbew_bool_t mbew_iterate(mbew_t m, mbew_num_t flags);

/* Returns MBEW_TRUE if the #mbew_t context is currently being iterated, MBEW_FALSE otherwise. */
MBEW_API mbew_bool_t mbew_iter_active(mbew_t m);

/* Returns the #mbew_data_t of the data bound to the current iteration. When MBEW_ITER_SYNC is used,
 * MBEW_DATA_NONE will be set as the context waits for the next frame to trigger. */
MBEW_API mbew_data_t mbew_iter_type(mbew_t m);

/* Returns the numeric frame index of the current iteration, beginning at 1. */
MBEW_API mbew_num_t mbew_iter_index(mbew_t m);

/* Returns the time, in nanoseconds, at which the currently iterated frame should be prosessed. */
MBEW_API mbew_ns_t mbew_iter_timestamp(mbew_t m);

/* When MBEW_ITER_SYNC is enabled, this function is used to both update the currently elapsed time
 * (in nanoseconds) AND, depending on whether TRUE or FALSE is returned, as an indicator of when
 * new, synchronized frame data is available to the caller. */
MBEW_API mbew_bool_t mbew_iter_sync(mbew_t m, mbew_ns_t elapsed);

/* This routine also works in conjuction with MBEW_ITER_SYNC and mbew_iter_sync(), returning the
 * amount of time until the next frame should be triggered. If BLOCKING code is permitted (for
 * example, if you have your iteration within a bakcground thread), sleeping or delaying by this
 * value is a perfectly feasible way of conserving resources. */
MBEW_API mbew_ns_t mbew_iter_next(mbew_t m);

/* If MBEW_ITER_FORMAT_RGB is set, this function will return a handle to that data. */
MBEW_API mbew_bytes_t mbew_iter_video_rgb(mbew_t m);

MBEW_API mbew_bytes_t* mbew_iter_video_yuv_planes(mbew_t m);
MBEW_API mbew_num_t* mbew_iter_video_yuv_stride(mbew_t m);

typedef enum _mbew_type_t {
	MBEW_TYPE_BOOL,
	MBEW_TYPE_SRC,
	MBEW_TYPE_STATUS,
	MBEW_TYPE_PROP,
	MBEW_TYPE_DATA,
	MBEW_TYPE_CODEC,
	MBEW_TYPE_ITER_BIT
} mbew_type_t;

/* Returns the string representation of an instance corresponding to the specified #mbew_type_t. */
MBEW_API const char* mbew_string(mbew_type_t type, ...);

#ifdef  __cplusplus
}
#endif

#endif

