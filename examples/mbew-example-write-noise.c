#include "mbew.h"

#include <stdio.h>
#include <stdlib.h>

static uint32_t mbew_noise_next(uint32_t* state) {
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;

	return *state;
}

int main(int argc, char** argv) {
	const mbew_write_config_t config = { 320, 180, 30 };
	const char* path = argc > 1 ? argv[1] : "mbew-noise.webm";
	size_t pixels_size = config.width * config.height * 4;
	uint8_t* pixels = (uint8_t*)(malloc(pixels_size));
	uint32_t state = 0x6D2B79F5;
	mbew_t writer;
	mbew_t reader;
	mbew_num_t frame;

	if(!pixels) return EXIT_FAILURE;
	writer = mbew_create(MBEW_SOURCE_WRITE_FILE, path, &config);
	if(!mbew_valid(writer)) {
		free(pixels);
		return EXIT_FAILURE;
	}

	for(frame = 0; frame < 90; frame++) {
		size_t pixel;

		for(pixel = 0; pixel < pixels_size; pixel += 4) {
			uint32_t noise = mbew_noise_next(&state) & 0x00F0F0F0;

			pixels[pixel] = (uint8_t)(noise);
			pixels[pixel + 1] = (uint8_t)(noise >> 8);
			pixels[pixel + 2] = (uint8_t)(noise >> 16);
			pixels[pixel + 3] = 255;
		}

		if(!mbew_write_frame(writer, pixels, config.width * 4)) break;
	}

	if(!mbew_write_finish(writer)) {
		mbew_destroy(writer);
		free(pixels);
		return EXIT_FAILURE;
	}

	mbew_destroy(writer);
	free(pixels);

	reader = mbew_create(MBEW_SOURCE_FILE, path);
	if(!mbew_valid(reader) ||
		mbew_property(reader, MBEW_PROPERTY_VIDEO_WIDTH).num != config.width ||
		mbew_property(reader, MBEW_PROPERTY_VIDEO_HEIGHT).num != config.height) {
		mbew_destroy(reader);
		return EXIT_FAILURE;
	}

	mbew_destroy(reader);
	return EXIT_SUCCESS;
}
