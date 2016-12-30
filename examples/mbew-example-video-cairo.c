#include "mbew.h"

#include <cairo.h>
#include <stdio.h>

void write_cairo(mbew_t m) {
	char filename[512];

	mbew_num_t width = mbew_property(m, MBEW_PROPERTY_VIDEO_WIDTH).num;
	mbew_num_t height = mbew_property(m, MBEW_PROPERTY_VIDEO_HEIGHT).num;
	mbew_bytes_t data = mbew_iter_rgb(m);

	cairo_surface_t* surface = cairo_image_surface_create_for_data(
		data,
		CAIRO_FORMAT_RGB24,
		width,
		height,
		cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width)
	);

	snprintf(filename, 512, "mbew-video-cairo-%03u.png", mbew_iter_index(m));
	printf("Writing: %s\n", filename);

	cairo_surface_write_to_png(surface, filename);
	cairo_surface_destroy(surface);
}

int main(int argc, char** argv) {
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		while(mbew_iterate(m, MBEW_ITERATE_RGB | MBEW_ITERATE_VIDEO)) write_cairo(m);
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}

