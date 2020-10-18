#ifndef MBEW_H
#define MBEW_H 1

#include <stdint.h>

#if defined(_MSC_VER)
	#define MBEW_API
	/* #define MBEW_API __declspec(dllimport) */
#else
	#define MBEW_API
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/* ======================================================================================= TYPES */

/* The private #mbew_t structure is the main context object within mbew, managing all the necessary
 * state for a single WebM source. To minimize dependencies--and to make libmbew seem as sleek and
 * sexy as possible--all of the implementation details are abstracted away within the sources. */
typedef struct _mbew_t* mbew_t;

/* A special type for nanoseconds, as they are commonly used. */
typedef uint64_t mbew_ns_t;

/* Rather than use all manner of disjoint numeric types, mbew settles on a single, simple type. */
typedef unsigned int mbew_num_t;

typedef double mbew_hz_t;
typedef unsigned char* mbew_bytes_t;

#define MBEW_ID 0xA7
#define MBEW_ID_SOURCE 0x01
#define MBEW_ID_STATUS 0x02
#define MBEW_ID_PROPERTY 0x03
#define MBEW_ID_DATA 0x04
#define MBEW_ID_CODEC 0x05
#define MBEW_ID_ITERATE 0x06

#define mbew_enum(id, val) ((MBEW_ID << 24) | (MBEW_ID_##id << 16) | val)
#define mbew_enum_id(e) ((e & 0x00FF0000) >> 16)
#define mbew_enum_value(e) (e & 0x0000FFFF)
#define mbew_enum_valid(e) (((e & 0xFF000000) >> 24 == MBEW_ID) && mbew_enum_id(e) <= 0x06)

#define MBEW_ENUM(id, name, val) MBEW_##id##_##name = mbew_enum(id, val)

/* Standard FALSE(zero) and TRUE (non-zero) values, usually preset somewhere in your include
 * hierarchy as simply TRUE and FALSE. */
typedef enum _mbew_tool_t {
	MBEW_FALSE,
	MBEW_TRUE
} mbew_bool_t;

typedef enum _mbew_source_t {
	/* Reads a WebM file from disk using the traditional fopen/fclose/etc. interface. A single
	 * argument is passed to mbew_create(), which is the path to file to be opened. */
	MBEW_ENUM(SOURCE, FILE, 0x00),

	/* Reads a WebM file from a buffer of memory. Two arguments are passed to mbew_create(): a
	 * void* pointer to the memory location and a size_t argument indicating its size. */
	MBEW_ENUM(SOURCE, MEMORY, 0x01)

	/* MBEW_ENUM(SOURCE, MMAP, 0x02) */
} mbew_source_t;

#define MBEW_STATUS_ID 0x0000
#define MBEW_STATUS_ID_SOURCE 0x0000
#define MBEW_STATUS_ID_NESTEGG 0x0000
#define MBEW_STATUS_ID_VPX 0x0000
#define MBEW_STATUS_ID_VORBIS 0x0000

#define MBEW_ENUM_STATUS(id, name, val) MBEW_ENUM(STATUS, id##_##name, MBEW_STATUS_ID_##id | val)

/* An #mbew_t context maintains an internal error tracking state that is reported to userspace via
 * the mbew_status() function. The #mbew_status_t enumerations represent to the various ways in
 * which things can sometimes fail internally. */
typedef enum _mbew_status_t {
	MBEW_ENUM(STATUS, VALID, MBEW_STATUS_ID | 0x00),
	MBEW_ENUM(STATUS, NOT_IMPLEMENTED, MBEW_STATUS_ID | 0x01),
	MBEW_ENUM(STATUS, NULL_CONTEXT, MBEW_STATUS_ID | 0x02),
	MBEW_ENUM(STATUS, ITERATE_FLAGS, MBEW_STATUS_ID | 0x03),

	MBEW_ENUM_STATUS(SOURCE, FILE, 0x00),
	MBEW_ENUM_STATUS(SOURCE, MEMORY, 0x01),

	MBEW_ENUM_STATUS(NESTEGG, INIT, 0x00),
	MBEW_ENUM_STATUS(NESTEGG, DURATION, 0x01),
	MBEW_ENUM_STATUS(NESTEGG, TSTAMP_SCALE, 0x02),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_COUNT, 0x03),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_TYPE, 0x04),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_VIDEO_PARAMS, 0x05),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_AUDIO_PARAMS, 0x06),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_CODEC_DATA_COUNT, 0x07),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_CODEC_DATA, 0x08),
	MBEW_ENUM_STATUS(NESTEGG, TRACK_SEEK, 0x09),
	MBEW_ENUM_STATUS(NESTEGG, OFFSET_SEEK, 0x0A),
	MBEW_ENUM_STATUS(NESTEGG, PACKET_TRACK, 0x0B),
	MBEW_ENUM_STATUS(NESTEGG, PACKET_COUNT, 0x0C),
	MBEW_ENUM_STATUS(NESTEGG, PACKET_TSTAMP, 0x0D),
	MBEW_ENUM_STATUS(NESTEGG, PACKET_DATA, 0x0E),
	MBEW_ENUM_STATUS(NESTEGG, READ_PACKET, 0x10),

	MBEW_ENUM_STATUS(VPX, CODEC_DEC_INIT, 0x00),
	MBEW_ENUM_STATUS(VPX, CODEC_DECODE, 0x01),
	MBEW_ENUM_STATUS(VPX, CODEC_GET_FRAME, 0x02),

	MBEW_ENUM_STATUS(VORBIS, HEADER_COUNT, 0x00),
	MBEW_ENUM_STATUS(VORBIS, SYNTHESIS_HEADERIN, 0x01),
	MBEW_ENUM_STATUS(VORBIS, SYNTHESIS_INIT, 0x02),
	MBEW_ENUM_STATUS(VORBIS, BLOCK_INIT, 0x03)
} mbew_status_t;

/* Properties represent the high-level attributes of a WebmM source. Each property can be accessed
 * using the #mbew_property_t enum directly OR by using the corresponding string name.
 *
 * NOTE: Take special care that you access the proper union member when fetching a property! */
typedef enum _mbew_property_t {
	/* The stream duration in nanoseconds.[key: duration] [val: mbew_ns_t] */
	MBEW_ENUM(PROPERTY, DURATION, 0x00),

	/* The scale for each timestamp value. [key: scale] [val: mbew_ns_t] */
	MBEW_ENUM(PROPERTY, SCALE, 0x01),

	/* The total number of tracks. [key: tracks] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, TRACKS, 0x02),

	/* True if a video track was found, false otherwise.* [key: video] [val: mbew_bool_t] */
	MBEW_ENUM(PROPERTY, VIDEO, 0x03),

	/* TODO: ...* [key: video.codec] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, VIDEO_CODEC, 0x04),

	/* The numeric index of the video track.* [key: video.track] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, VIDEO_TRACK, 0x05),

	/* Pixel-width of the video. [key: video.width] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, VIDEO_WIDTH, 0x06),

	/* Pixel-height of the video. [key: video.width] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, VIDEO_HEIGHT, 0x07),

	/* True if a audio track was found, false otherwise. [key: audio] [val: mbew_bool_t] */
	MBEW_ENUM(PROPERTY, AUDIO, 0x08),

	/* The numeric index of the audio track. [key: audio.track] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, AUDIO_TRACK, 0x09),

	/* TODO: ... [key: audio.codec] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, AUDIO_CODEC, 0x0A),

	/* Sampling rate in HZ. [key: audio.rate] [val: mbew_hz_t] */
	MBEW_ENUM(PROPERTY, AUDIO_RATE, 0x0B),

	/* Number of audio channels. [key: audio.channels] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, AUDIO_CHANNELS, 0x0C),

	/* Bits per sample. [key: audio.depth] [val: mbew_num_t] */
	MBEW_ENUM(PROPERTY, AUDIO_DEPTH, 0x0D)
} mbew_property_t;

/* The #mbew_data_t enum is used to signify to client code what "type" of data is currently being
 * yielded during iteration. The MBEW_DATA_SYNC value is only encountered when the MBEW_ITERATE_SYNC
 * #mbew_iterate_t is set, and is set when the caller is not synchronized with the iteration. */
typedef enum _mbew_data_t {
	MBEW_ENUM(DATA, SYNC, 0x00),
	MBEW_ENUM(DATA, VIDEO, 0x01),
	MBEW_ENUM(DATA, AUDIO, 0x02)
} mbew_data_t;

/* WebM currently only supports the following 4 codecs: 2 video, 2 audio. */
typedef enum _mbew_codec_t {
	MBEW_ENUM(CODEC, VP8, 0x00),
	MBEW_ENUM(CODEC, VORBIS, 0x01),
	MBEW_ENUM(CODEC, VP9, 0x02),
	MBEW_ENUM(CODEC, OPUS, 0x03)
} mbew_codec_t;

/* The #mbew_iterate_t values are used to control the behavior of the mbew_iterate() function. Some
 * of the values are mutually-exclusive; see the specific comments for additional details. */
typedef enum _mbew_iterate_t {
	/* Only MBEW_DATA_VIDEO (and MBEW_DATA_SYNC if necessary) data is exposed to the caller during
	 * the stream iteration. */
	MBEW_ENUM(ITERATE, VIDEO, 1),

	/* This value works on audio data just as MBEW_ITERATE_VIDEO does for video. The two fields
	 * CANNOT be used in conjuction; doing so will cause mbew_iterate() to error. */
	MBEW_ENUM(ITERATE, AUDIO, 1 << 1),

	/* When this flag is specified, mbew_iterate()'s behavior is expanded to begin using time data,
	 * provided by the caller via the mbew_iter_sync() function, to help determine whether it should
	 * advance to the next available frame for processing or simply yield a kind of "no-op" state,
	 * essentially informing the caller that they should "try again later." This provides the caller
	 * additional flexibility in situations where blocking code is undesired, but where
	 * mbew_iterate() will likely be called MUCH faster than the sources actual framerate.
	 *
	 * When MBEW_ITERATE_SYNC is enabled, the #mbew_t iteration state relies on the caller to provide
	 * time information using the mbew_iter_sync() function. Once the "elapsed" time equals or
	 * exceeds the timestamp of the current iterations queued up frame, mbew_iter_sync() will return
	 * MBEW_TRUE, informing the caller that the next frame is currently bound to the current
	 * iteration and can be accessed safely. */
	MBEW_ENUM(ITERATE, SYNC, 1 << 2),

	/* By default, video data is accessed using the mbew_iter_yuv() functions (which prevents
	 * libmbew from having to do potentially costly per-frame format conversion). However, if the
	 * caller needs access to standard RGB data instead, using this flag will instruct the
	 * interation process to composite all of the YUV420 data into a typical 4-byte RGBA (with alpha
	 * being unused) color buffer. This data is accessed via mbew_iter_rgb(). */
	MBEW_ENUM(ITERATE, RGB, 1 << 3),
} mbew_iterate_t;

/* This union encapsulates each kind of single value that can be returned as property. The
 * #mbew_property_t enums above each indicate (using the [val: mbew_num_t] syntax) what low-level type
 * is used to hold its data, and userspace could should make sure to access the appropriate member
 * for the given property. */
typedef union _mbew_propval_t {
	mbew_ns_t ns;
	mbew_num_t num;
	mbew_hz_t hz;
	mbew_bool_t b;
} mbew_propval_t;

/* ========================================================================================= API */

/* Creates a new #mbew_t context for the given source type. Depending on the #mbew_source_t enumeration
 * specified, one ore more variable arguments will be necessary; see the #mbew_source_t documentation
 * for more info.
 *
 * To determine whether or not the context was created successfully, use the mbew_status() function,
 * as simply relying on the #mbew_t context itself to be non-NULL is not sufficient. */
MBEW_API mbew_t mbew_create(mbew_source_t src, ...);

/* Destroys a previously created context. An #mbew_t context that is currently being iterated over
 * (which you determine using mbew_iter_active()) cannot be destroyed until it either finishes it
 * iteration and or is manually reset via mbew_reset(). */
MBEW_API void mbew_destroy(mbew_t m);

/* Resets all audio, video, and iteration state back to their defaults. Returns MBEW_TRUE if all of
 * the reset operations completed successfully, MBEW_FALSE otherwise. Additional details on failure
 * can be queried using mbew_status(). */
MBEW_API mbew_bool_t mbew_reset(mbew_t m);

/* Returns the most recent status associated with the given context. */
MBEW_API mbew_status_t mbew_status(mbew_t m);

#define mbew_valid(m) (mbew_status(m) == MBEW_STATUS_VALID)

/* Accepts either a single #mbew_property_t value OR string, returning the resultant value
 * encapsulated within a #mbew_propval_t union. */
MBEW_API mbew_propval_t mbew_property(mbew_t m, ...);

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

/* Returns the #mbew_data_t of the data bound to the current iteration. When MBEW_ITERATE_SYNC is used,
 * MBEW_DATA_SYNC will be set as the context waits for the next frame to trigger. */
MBEW_API mbew_data_t mbew_iter_type(mbew_t m);

/* Returns the numeric frame index of the current iteration, beginning at 1. */
MBEW_API mbew_num_t mbew_iter_index(mbew_t m);

/* Returns the time, in nanoseconds, at which the currently iterated frame should be prosessed. */
MBEW_API mbew_ns_t mbew_iter_timestamp(mbew_t m);

/* When MBEW_ITERATE_SYNC is enabled, this function is used to both update the currently elapsed time
 * (in nanoseconds) AND, depending on whether TRUE or FALSE is returned, as an indicator of when
 * new, synchronized frame data is available to the caller. */
MBEW_API mbew_bool_t mbew_iter_sync(mbew_t m, mbew_ns_t elapsed);

/* This routine also works in conjuction with MBEW_ITERATE_SYNC and mbew_iter_sync(), returning the
 * amount of time until the next frame should be triggered. If BLOCKING code is permitted (for
 * example, if you have your iteration within a bakcground thread), sleeping or delaying by this
 * value is a perfectly feasible way of conserving resources. */
MBEW_API mbew_ns_t mbew_iter_next(mbew_t m);

/* If MBEW_ITERATE_RGB is set, this function will return a handle to that data. */
MBEW_API mbew_bytes_t mbew_iter_rgb(mbew_t m);

MBEW_API mbew_bytes_t* mbew_iter_yuv_planes(mbew_t m);
MBEW_API mbew_num_t* mbew_iter_yuv_stride(mbew_t m);

MBEW_API const int16_t* mbew_iter_pcm16(mbew_t m);
MBEW_API mbew_num_t mbew_iter_pcm16_size(mbew_t m);

/* Returns descriptive information regarding the enum instance passed in as $e. */
MBEW_API const char* mbew_string(mbew_num_t e);

#ifdef  __cplusplus
}
#endif

#endif

