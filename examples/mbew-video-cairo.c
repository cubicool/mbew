#include "mbew.h"

#include <cairo.h>
#include <stdio.h>

void video_cairo_callback(mbew_iter_t type, mbew_num_t index, mbew_ns_t tstamp, void* data) {
	char filename[512];

	if(type == MBEW_ITER_VIDEO) {
		mbew_data_video_t* video_data = (mbew_data_video_t*)(data);

		cairo_surface_t* surface = cairo_image_surface_create_for_data(
			video_data->data,
			CAIRO_FORMAT_RGB24,
			video_data->width,
			video_data->height,
			cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, video_data->width)
		);

		snprintf(filename, 512, "mbew-video-cairo-%03u.png", index);

		cairo_surface_write_to_png(surface, filename);
		cairo_surface_destroy(surface);
	}
}

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(mbew))) {
		printf("Opened mbew context for '%s'\n", argv[1]);

		mbew_iterate(mbew, video_cairo_callback);
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(mbew);

	return 0;
}

