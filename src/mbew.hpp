#ifndef MBEW_HPP
#define MBEW_HPP 1

#include "mbew.h"

#include <string>
#include <memory>

namespace mbew {

typedef mbew_ns_t ns_t;
typedef mbew_num_t num_t;
typedef mbew_hz_t hz_t;
typedef mbew_bytes_t bytes_t;
typedef mbew_propval_t propval_t;

enum class Status {
	VALID = MBEW_STATUS_VALID,
	NOT_IMPLEMENTED = MBEW_STATUS_NOT_IMPLEMENTED,
	NULL_CONTEXT = MBEW_STATUS_NULL_CONTEXT,
	ITERATE_FLAGS = MBEW_STATUS_ITERATE_FLAGS,
	SOURCE_FILE = MBEW_STATUS_SOURCE_FILE,
	SOURCE_MEMORY = MBEW_STATUS_SOURCE_MEMORY,
	NESTEGG_INIT = MBEW_STATUS_NESTEGG_INIT,
	NESTEGG_DURATION = MBEW_STATUS_NESTEGG_DURATION,
	NESTEGG_TSTAMP_SCALE = MBEW_STATUS_NESTEGG_TSTAMP_SCALE,
	NESTEGG_TRACK_COUNT = MBEW_STATUS_NESTEGG_TRACK_COUNT,
	NESTEGG_TRACK_TYPE = MBEW_STATUS_NESTEGG_TRACK_TYPE,
	NESTEGG_TRACK_VIDEO_PARAMS = MBEW_STATUS_NESTEGG_TRACK_VIDEO_PARAMS,
	NESTEGG_TRACK_AUDIO_PARAMS = MBEW_STATUS_NESTEGG_TRACK_AUDIO_PARAMS,
	NESTEGG_TRACK_CODEC_DATA_COUNT = MBEW_STATUS_NESTEGG_TRACK_CODEC_DATA_COUNT,
	NESTEGG_TRACK_CODEC_DATA = MBEW_STATUS_NESTEGG_TRACK_CODEC_DATA,
	NESTEGG_TRACK_SEEK = MBEW_STATUS_NESTEGG_TRACK_SEEK,
	NESTEGG_OFFSET_SEEK = MBEW_STATUS_NESTEGG_OFFSET_SEEK,
	NESTEGG_PACKET_TRACK = MBEW_STATUS_NESTEGG_PACKET_TRACK,
	NESTEGG_PACKET_COUNT = MBEW_STATUS_NESTEGG_PACKET_COUNT,
	NESTEGG_PACKET_TSTAMP = MBEW_STATUS_NESTEGG_PACKET_TSTAMP,
	NESTEGG_PACKET_DATA = MBEW_STATUS_NESTEGG_PACKET_DATA,
	NESTEGG_READ_PACKET = MBEW_STATUS_NESTEGG_READ_PACKET,
	VPX_CODEC_DEC_INIT = MBEW_STATUS_VPX_CODEC_DEC_INIT,
	VPX_CODEC_DECODE = MBEW_STATUS_VPX_CODEC_DECODE,
	VPX_CODEC_GET_FRAME = MBEW_STATUS_VPX_CODEC_GET_FRAME,
	VORBIS_HEADER_COUNT = MBEW_STATUS_VORBIS_HEADER_COUNT,
	VORBIS_SYNTHESIS_HEADERIN = MBEW_STATUS_VORBIS_SYNTHESIS_HEADERIN,
	VORBIS_SYNTHESIS_INIT = MBEW_STATUS_VORBIS_SYNTHESIS_INIT,
	VORBIS_BLOCK_INIT = MBEW_STATUS_VORBIS_BLOCK_INIT
};

enum class Property {
	DURATION = MBEW_PROPERTY_DURATION,
	SCALE = MBEW_PROPERTY_SCALE,
	TRACKS = MBEW_PROPERTY_TRACKS,
	VIDEO = MBEW_PROPERTY_VIDEO,
	VIDEO_TRACK = MBEW_PROPERTY_VIDEO_TRACK,
	VIDEO_WIDTH = MBEW_PROPERTY_VIDEO_WIDTH,
	VIDEO_HEIGHT = MBEW_PROPERTY_VIDEO_HEIGHT,
	AUDIO = MBEW_PROPERTY_AUDIO,
	AUDIO_TRACK = MBEW_PROPERTY_AUDIO_TRACK,
	AUDIO_RATE = MBEW_PROPERTY_AUDIO_RATE,
	AUDIO_CHANNELS = MBEW_PROPERTY_AUDIO_CHANNELS,
	AUDIO_DEPTH = MBEW_PROPERTY_AUDIO_DEPTH
};

enum class Data {
	NONE = MBEW_DATA_SYNC,
	VIDEO = MBEW_DATA_VIDEO,
	AUDIO = MBEW_DATA_AUDIO
};

enum class Codec {
	VP8 = MBEW_CODEC_VP8,
	VORBIS = MBEW_CODEC_VORBIS,
	VP9 = MBEW_CODEC_VP9,
	OPUS = MBEW_CODEC_OPUS
};

namespace Iterate { enum {
	VIDEO = MBEW_ITERATE_VIDEO,
	AUDIO = MBEW_ITERATE_AUDIO,
	SYNC = MBEW_ITERATE_SYNC,
	RGB = MBEW_ITERATE_RGB
}; }

namespace impl { class context_t; }

typedef std::shared_ptr<impl::context_t> Context;

Context create(const std::string& path);

namespace impl {

class context_t {
public:
	~context_t() {
		mbew_destroy(_m);
	}

	bool reset() {
		return mbew_reset(_m);
	}

	Status status() const {
		return static_cast<Status>(mbew_status(_m));
	}

	bool valid() const {
		return mbew_valid(_m);
	}

	propval_t property(Property prop) const {
		return mbew_property(_m, prop);
	}

	bool iterate(num_t flags = 0) {
		return mbew_iterate(_m, flags);
	}

	struct Iter {
		Iter(const mbew_t* m):
		_m(m) {
		}

		bool active() const {
			return mbew_iter_active(*_m);
		}

		Data type() const {
			return static_cast<Data>(mbew_iter_type(*_m));
		}

		num_t index() const {
			return mbew_iter_index(*_m);
		}

		ns_t timestamp() const {
			return mbew_iter_timestamp(*_m);
		}

		bool sync(ns_t elapsed) const {
			return mbew_iter_sync(*_m, elapsed);
		}

		ns_t next() const {
			return mbew_iter_next(*_m);
		}

		bytes_t rgb() const {
			return mbew_iter_rgb(*_m);
		}

		bytes_t* yuv_planes() const {
			return mbew_iter_yuv_planes(*_m);
		}

		num_t* yuv_stride() const {
			return mbew_iter_yuv_stride(*_m);
		}

	private:
		const mbew_t* _m;
	};

	const Iter iter;

protected:
	friend Context mbew::create(const std::string& path);

	context_t(const std::string& path):
	iter(&_m) {
		_m = mbew_create(MBEW_SOURCE_FILE, path.c_str());
	}

	mbew_t _m;
};

}

template<typename Enum>
std::string string(Enum e) {
	return mbew_string(static_cast<num_t>(e));
}

Context create(const std::string& path) {
	return Context(new impl::context_t(path));
}

}

#endif

