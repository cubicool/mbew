#include "mbew-private.h"

#include "vpx/vp8cx.h"

#include <string.h>
#include <limits.h>
#include <stdlib.h>

static mbew_bool_t mbew_write_bytes(mbew_t m, const void* data, size_t size) {
	return fwrite(data, 1, size, m->writer.file) == size ? MBEW_TRUE : MBEW_FALSE;
}

static mbew_bool_t mbew_write_id(mbew_t m, uint32_t id) {
	uint8_t data[4];
	size_t size = id > 0xFFFFFF ? 4 : id > 0xFFFF ? 3 : id > 0xFF ? 2 : 1;
	size_t i;

	for(i = 0; i < size; i++) data[size - i - 1] = (uint8_t)(id >> (i * 8));

	return mbew_write_bytes(m, data, size);
}

static mbew_bool_t mbew_write_size(mbew_t m, uint64_t value) {
	uint8_t data[8];
	size_t size = 1;
	size_t i;

	while(size < 8 && value >= ((UINT64_C(1) << (size * 7)) - 1)) size++;

	if(size == 8 && value >= UINT64_C(0x00FFFFFFFFFFFFFF)) return MBEW_FALSE;

	for(i = 0; i < size; i++) data[size - i - 1] = (uint8_t)(value >> (i * 8));

	data[0] |= (uint8_t)(UINT64_C(1) << (8 - size));

	return mbew_write_bytes(m, data, size);
}

static mbew_bool_t mbew_write_uint(mbew_t m, uint32_t id, uint64_t value) {
	uint8_t data[8];
	size_t size = 1;
	size_t i;

	while(size < 8 && value >= (UINT64_C(1) << (size * 8))) size++;
	for(i = 0; i < size; i++) data[size - i - 1] = (uint8_t)(value >> (i * 8));

	return mbew_write_id(m, id) && mbew_write_size(m, size) && mbew_write_bytes(m, data, size);
}

static mbew_bool_t mbew_write_string(mbew_t m, uint32_t id, const char* value) {
	size_t size = strlen(value);

	return mbew_write_id(m, id) && mbew_write_size(m, size) && mbew_write_bytes(m, value, size);
}

static mbew_bool_t mbew_write_master(mbew_t m, uint32_t id, uint64_t size) {
	return mbew_write_id(m, id) && mbew_write_size(m, size);
}

static mbew_bool_t mbew_write_header(mbew_t m) {
	uint8_t ebml[] = {
		0x1A, 0x45, 0xDF, 0xA3, 0x9F,
		0x42, 0x86, 0x81, 0x01, 0x42, 0xF7, 0x81, 0x01,
		0x42, 0xF2, 0x81, 0x04, 0x42, 0xF3, 0x81, 0x08,
		0x42, 0x82, 0x84, 'w', 'e', 'b', 'm',
		0x42, 0x87, 0x81, 0x02, 0x42, 0x85, 0x81, 0x02,
		0x18, 0x53, 0x80, 0x67, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};
	uint8_t duration[8] = { 0 };
	uint64_t default_duration = UINT64_C(1000000000) / m->writer.config.rate;
	uint64_t video_size;
	uint64_t entry_size;

	if(!mbew_write_bytes(m, ebml, sizeof(ebml))) return MBEW_FALSE;
	if(!mbew_write_master(m, 0x1549A966, 32)) return MBEW_FALSE;
	if(!mbew_write_uint(m, 0x2AD7B1, 1000000)) return MBEW_FALSE;
	if(!mbew_write_string(m, 0x4D80, "mbew")) return MBEW_FALSE;
	if(!mbew_write_string(m, 0x5741, "mbew")) return MBEW_FALSE;
	if(!mbew_write_id(m, 0x4489) || !mbew_write_size(m, sizeof(duration))) return MBEW_FALSE;

	m->writer.duration_offset = ftell(m->writer.file);

	if(!mbew_write_bytes(m, duration, sizeof(duration))) return MBEW_FALSE;

	video_size = 1 + 1 + 2 + 1 + 1 + 2;
	entry_size = 3 + 4 + 3 + 3 + 7 + 8 + 2 + video_size;

	if(!mbew_write_master(m, 0x1654AE6B, entry_size + 2)) return MBEW_FALSE;
	if(!mbew_write_master(m, 0xAE, entry_size)) return MBEW_FALSE;
	if(!mbew_write_uint(m, 0xD7, 1) || !mbew_write_uint(m, 0x73C5, 1)) return MBEW_FALSE;
	if(!mbew_write_uint(m, 0x83, 1) || !mbew_write_uint(m, 0x9C, 0)) return MBEW_FALSE;
	if(!mbew_write_string(m, 0x86, "V_VP8")) return MBEW_FALSE;
	if(!mbew_write_uint(m, 0x23E383, default_duration)) return MBEW_FALSE;
	if(!mbew_write_master(m, 0xE0, video_size)) return MBEW_FALSE;

	return
		mbew_write_uint(m, 0xB0, m->writer.config.width) &&
		mbew_write_uint(m, 0xBA, m->writer.config.height)
	;
}

mbew_bool_t mbew_writer_create(mbew_t m, const char* path, const mbew_write_config_t* config) {
	vpx_codec_enc_cfg_t codec_config;

	if(
		!path || !config ||
		!config->width || !config->height || !config->rate ||
		config->rate > INT_MAX ||
		(config->width & 1) || (config->height & 1)
	) {
		m->status = MBEW_STATUS_WRITE_INVALID;

		return MBEW_FALSE;
	}

	m->writer.file = fopen(path, "wb");

	if(!m->writer.file) goto io_fail;

	m->writer.config = *config;

	if(vpx_codec_enc_config_default(&vpx_codec_vp8_cx_algo, &codec_config, 0)) goto invalid_fail;

	codec_config.g_w = config->width;
	codec_config.g_h = config->height;
	codec_config.g_timebase.num = 1;
	codec_config.g_timebase.den = (int)config->rate;
	codec_config.g_lag_in_frames = 0;

	if(vpx_codec_enc_init(&m->writer.codec, &vpx_codec_vp8_cx_algo, &codec_config, 0)) goto codec_fail;

	m->writer.codec_init = MBEW_TRUE;

	if(!vpx_img_alloc(&m->writer.image, VPX_IMG_FMT_I420, config->width, config->height, 1)) goto codec_fail;

	m->writer.image_init = MBEW_TRUE;

	if(!mbew_write_header(m)) goto io_fail;

	m->writer.init = MBEW_TRUE;

	return MBEW_TRUE;

codec_fail:
	m->status = MBEW_STATUS_VPX_CODEC_ENC_INIT;

	goto fail;

invalid_fail:
	m->status = MBEW_STATUS_WRITE_INVALID;

	goto fail;

io_fail:
	m->status = MBEW_STATUS_WRITE_IO;

fail:
	if(m->writer.image_init) vpx_img_free(&m->writer.image);
	if(m->writer.codec_init) vpx_codec_destroy(&m->writer.codec);
	if(m->writer.file) fclose(m->writer.file);

	m->writer.file = NULL;

	return MBEW_FALSE;
}

static mbew_bool_t mbew_write_packet(mbew_t m, const vpx_codec_cx_pkt_t* packet) {
	uint64_t timestamp;
	uint8_t block[4] = { 0x81, 0, 0, 0 };
	int16_t relative;

	if(packet->data.frame.pts < 0) return MBEW_FALSE;

	timestamp = (uint64_t)packet->data.frame.pts * 1000 / m->writer.config.rate;

	if(!m->writer.frames || timestamp - m->writer.cluster_timestamp >= 30000) {
		m->writer.cluster_timestamp = timestamp;

		if(!mbew_write_id(m, 0x1F43B675) || !mbew_write_bytes(m, "\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 8)) return MBEW_FALSE;
		if(!mbew_write_uint(m, 0xE7, timestamp)) return MBEW_FALSE;
	}

	relative = (int16_t)(timestamp - m->writer.cluster_timestamp);
	block[1] = (uint8_t)((uint16_t)(relative) >> 8);
	block[2] = (uint8_t)(relative);
	block[3] = (uint8_t)(packet->data.frame.flags & VPX_FRAME_IS_KEY ? 0x80 : 0);

	return
		mbew_write_id(m, 0xA3) &&
		mbew_write_size(m, packet->data.frame.sz + sizeof(block)) &&
		mbew_write_bytes(m, block, sizeof(block)) &&
		mbew_write_bytes(m, packet->data.frame.buf, packet->data.frame.sz)
	;
}

static mbew_bool_t mbew_write_drain(mbew_t m) {
	vpx_codec_iter_t iter = NULL;
	const vpx_codec_cx_pkt_t* packet;

	while((packet = vpx_codec_get_cx_data(&m->writer.codec, &iter))) {
		if(
			packet->kind == VPX_CODEC_CX_FRAME_PKT &&
			!mbew_write_packet(m, packet)
		) return MBEW_FALSE;
	}

	return MBEW_TRUE;
}

mbew_bool_t mbew_write_frame(mbew_t m, const void* rgba, size_t stride) {
	const uint8_t* src = (const uint8_t*)(rgba);
	mbew_num_t x;
	mbew_num_t y;
	ptrdiff_t ystride;
	ptrdiff_t ustride;
	ptrdiff_t vstride;

	if(
		!m || m->src != MBEW_SOURCE_WRITE_FILE ||
		!m->writer.init || m->writer.finished || !src ||
		stride < (size_t)m->writer.config.width * 4
	) return MBEW_FALSE;

	ystride = m->writer.image.stride[VPX_PLANE_Y];
	ustride = m->writer.image.stride[VPX_PLANE_U];
	vstride = m->writer.image.stride[VPX_PLANE_V];

	for(y = 0; y < m->writer.config.height; y++) for(x = 0; x < m->writer.config.width; x++) {
		const uint8_t* pixel = src + y * stride + x * 4;

		m->writer.image.planes[VPX_PLANE_Y][(ptrdiff_t)y * ystride + (ptrdiff_t)x] =
			(uint8_t)(((66 * pixel[0] + 129 * pixel[1] + 25 * pixel[2] + 128) >> 8) + 16)
		;
	}

	for(y = 0; y < m->writer.config.height; y += 2) for(x = 0; x < m->writer.config.width; x += 2) {
		int u = 0;
		int v = 0;
		mbew_num_t dx;
		mbew_num_t dy;

		for(dy = 0; dy < 2; dy++) for(dx = 0; dx < 2; dx++) {
			const uint8_t* pixel = src + (y + dy) * stride + (x + dx) * 4;
			u += (-38 * pixel[0] - 74 * pixel[1] + 112 * pixel[2] + 128) >> 8;
			v += (112 * pixel[0] - 94 * pixel[1] - 18 * pixel[2] + 128) >> 8;
		}

		m->writer.image.planes[VPX_PLANE_U][(ptrdiff_t)(y / 2) * ustride + (ptrdiff_t)(x / 2)] = (uint8_t)(u / 4 + 128);
		m->writer.image.planes[VPX_PLANE_V][(ptrdiff_t)(y / 2) * vstride + (ptrdiff_t)(x / 2)] = (uint8_t)(v / 4 + 128);
	}

	if(vpx_codec_encode(&m->writer.codec, &m->writer.image, m->writer.frames, 1, 0, VPX_DL_REALTIME)) goto fail;
	if(!mbew_write_drain(m)) goto io_fail;

	m->writer.frames++;

	return MBEW_TRUE;

io_fail:
	m->status = MBEW_STATUS_WRITE_IO;

	return MBEW_FALSE;

fail:
	m->status = MBEW_STATUS_VPX_CODEC_ENCODE;

	return MBEW_FALSE;
}

mbew_bool_t mbew_write_finish(mbew_t m) {
	union { double value; uint8_t data[8]; } duration;
	uint8_t output[8];
	size_t i;

	if(!m || m->src != MBEW_SOURCE_WRITE_FILE || !m->writer.init) return MBEW_FALSE;
	if(m->writer.finished) return MBEW_TRUE;
	if(vpx_codec_encode(&m->writer.codec, NULL, 0, 0, 0, VPX_DL_REALTIME) || !mbew_write_drain(m)) goto fail;

	duration.value = (double)(m->writer.frames) * 1000.0 / m->writer.config.rate;

	for(i = 0; i < sizeof(output); i++) output[i] = duration.data[sizeof(output) - i - 1];

	if(fseek(m->writer.file, m->writer.duration_offset, SEEK_SET) || !mbew_write_bytes(m, output, sizeof(output)) || fclose(m->writer.file)) goto fail;

	m->writer.file = NULL;

	vpx_img_free(&m->writer.image);
	vpx_codec_destroy(&m->writer.codec);

	m->writer.image_init = MBEW_FALSE;
	m->writer.codec_init = MBEW_FALSE;
	m->writer.finished = MBEW_TRUE;

	return MBEW_TRUE;

fail:
	m->status = MBEW_STATUS_WRITE_IO;

	return MBEW_FALSE;
}

void mbew_writer_destroy(mbew_t m) {
	if(m->writer.init && !m->writer.finished) mbew_write_finish(m);
	if(m->writer.image_init) vpx_img_free(&m->writer.image);
	if(m->writer.codec_init) vpx_codec_destroy(&m->writer.codec);
	if(m->writer.file) fclose(m->writer.file);

	free(m);
}

mbew_propval_t mbew_writer_property(mbew_t m, mbew_property_t prop) {
	mbew_propval_t r = { 0 };

	switch(prop) {
		case MBEW_PROPERTY_DURATION: r.ns = m->writer.frames * UINT64_C(1000000000) / m->writer.config.rate; break;
		case MBEW_PROPERTY_SCALE: r.ns = 1000000; break;
		case MBEW_PROPERTY_TRACKS: r.num = 1; break;
		case MBEW_PROPERTY_VIDEO: r.b = MBEW_TRUE; break;
		case MBEW_PROPERTY_VIDEO_TRACK: r.num = 0; break;
		case MBEW_PROPERTY_VIDEO_CODEC: r.num = (mbew_num_t)MBEW_CODEC_VP8; break;
		case MBEW_PROPERTY_VIDEO_WIDTH: r.num = m->writer.config.width; break;
		case MBEW_PROPERTY_VIDEO_HEIGHT: r.num = m->writer.config.height; break;
		default: break;
	}

	return r;
}
