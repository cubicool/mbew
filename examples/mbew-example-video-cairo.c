#include "mbew.h"

#include <cairo.h>
#include <stdio.h>

void iter_to_cairo(mbew_iter_t* iter) {
	char filename[512];

	if(mbew_iter_type(iter) == MBEW_DATA_VIDEO) {
		mbew_data_video_t* data = mbew_iter_video(iter);

		cairo_surface_t* surface = cairo_image_surface_create_for_data(
			data->data.rgb,
			CAIRO_FORMAT_RGB24,
			data->width,
			data->height,
			cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, data->width)
		);

		snprintf(filename, 512, "mbew-video-cairo-%03u.png", mbew_iter_index(iter));

		cairo_surface_write_to_png(surface, filename);
		cairo_surface_destroy(surface);
	}
}

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(mbew))) {
		mbew_iter_t* iter = NULL;

		while(mbew_iterate(mbew, &iter, MBEW_ITER_FORMAT_RGB)) iter_to_cairo(iter);

		mbew_iter_destroy(iter);
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(mbew);

	return 0;
}

