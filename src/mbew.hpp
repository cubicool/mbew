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
	NULL_CONTEXT = MBEW_STATUS_NULL_CONTEXT,
	SOURCE_FILE = MBEW_STATUS_SOURCE_FILE,
	SOURCE_MEMORY = MBEW_STATUS_SOURCE_MEMORY,
	INIT_IO = MBEW_STATUS_INIT_IO,
	INIT_CODEC = MBEW_STATUS_INIT_CODEC,
	DURATION = MBEW_STATUS_DURATION,
	SCALE = MBEW_STATUS_SCALE,
	TRACK_COUNT = MBEW_STATUS_TRACK_COUNT,
	UNKNOWN_TRACK = MBEW_STATUS_UNKNOWN_TRACK,
	PARAMS_VIDEO = MBEW_STATUS_PARAMS_VIDEO,
	PARAMS_AUDIO = MBEW_STATUS_PARAMS_AUDIO,
	PACKET_READ = MBEW_STATUS_PACKET_READ,
	PACKET_TRACK = MBEW_STATUS_PACKET_TRACK,
	PACKET_COUNT = MBEW_STATUS_PACKET_COUNT,
	PACKET_TSTAMP = MBEW_STATUS_PACKET_TSTAMP,
	PACKET_DURATION = MBEW_STATUS_PACKET_DURATION,
	PACKET_DATA = MBEW_STATUS_PACKET_DATA,
	VPX_DECODE = MBEW_STATUS_VPX_DECODE,
	GET_FRAME = MBEW_STATUS_GET_FRAME,
	SEEK_OFFSET = MBEW_STATUS_SEEK_OFFSET,
	SEEK_VIDEO = MBEW_STATUS_SEEK_VIDEO,
	SEEK_AUDIO = MBEW_STATUS_SEEK_AUDIO,
	ITERATE_FLAGS = MBEW_STATUS_ITERATE_FLAGS,
	NOT_IMPLEMENTED = MBEW_STATUS_NOT_IMPLEMENTED
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

