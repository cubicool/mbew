#include "mbew.h"

#include <cairo.h>
#include <stdio.h>

void write_cairo(mbew_t m) {
	char filename[512];

	mbew_num_t width = mbew_property(m, MBEW_PROP_VIDEO_WIDTH).num;
	mbew_num_t height = mbew_property(m, MBEW_PROP_VIDEO_HEIGHT).num;
	mbew_bytes_t data = mbew_iter_video_rgb(m);

	cairo_surface_t* surface = cairo_image_surface_create_for_data(
		data,
		CAIRO_FORMAT_RGB24,
		width,
		height,
		cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width)
	);

	snprintf(filename, 512, "mbew-video-cairo-%03u.png", mbew_iter_index(m));

	cairo_surface_write_to_png(surface, filename);
	cairo_surface_destroy(surface);
}

int main(int argc, char** argv) {
	mbew_t m = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(m))) {
		while(mbew_iterate(m, MBEW_ITER_FORMAT_RGB | MBEW_ITER_VIDEO_ONLY)) write_cairo(m);
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(m);

	return 0;
}

