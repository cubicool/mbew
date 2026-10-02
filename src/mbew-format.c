#include "mbew-private.h"

mbew_num_t mbew_video_frame_size(mbew_t m, mbew_num_t flags) {
	mbew_num_t width;
	mbew_num_t height;

	mbew_num_t luma;
	mbew_num_t chroma;

	if(!m) return 0;

	if(m->src == MBEW_SOURCE_WRITE_FILE) {
		width = m->writer.config.width;
		height = m->writer.config.height;
	}

	else {
		if(!m->video.track.init) return 0;
		width = m->video.params.width;
		height = m->video.params.height;
	}

	if(mbew_flags(flags, MBEW_ITERATE_RGB)) return width * height * 4;

	/* YUV420: one full-resolution Y plane, plus two quarter-resolution U/V planes. Round the
	 * chroma dimensions up to handle odd width/height, matching libvpx's own I420 layout. */
	luma = width * height;
	chroma = ((width + 1) / 2) * ((height + 1) / 2);

	return luma + (2 * chroma);
}

/* http://stackoverflow.com/questions/29327705/libvpx-convert-vpx-img-fmt-i420-rgb */
void mbew_format_rgb(vpx_image_t* img, uint8_t* dest) {
	mbew_num_t width = img->d_w;
	mbew_num_t height = img->d_h;
	const uint8_t* y = img->planes[VPX_PLANE_Y];
	const uint8_t* u = img->planes[VPX_PLANE_U];
	const uint8_t* v = img->planes[VPX_PLANE_V];
	ptrdiff_t ystride = img->stride[VPX_PLANE_Y];
	ptrdiff_t ustride = img->stride[VPX_PLANE_U];
	ptrdiff_t vstride = img->stride[VPX_PLANE_V];

	size_t i;
	size_t j;

	for(i = 0; i < height; ++i) {
		for(j = 0; j < width; ++j) {
			int r;
			int g;
			int b;

			uint8_t* point = dest + 4 * ((i * (size_t)width) + j);

			int t_y = y[(ptrdiff_t)i * ystride + (ptrdiff_t)j];
			int t_u = u[(ptrdiff_t)(i / 2) * ustride + (ptrdiff_t)(j / 2)];
			int t_v = v[(ptrdiff_t)(i / 2) * vstride + (ptrdiff_t)(j / 2)];

			t_y = t_y < 16 ? 16 : t_y;

			r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
			g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
			b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;

			point[2] = (uint8_t)(r > 255 ? 255 : r < 0 ? 0 : r);
			point[1] = (uint8_t)(g > 255 ? 255 : g < 0 ? 0 : g);
			point[0] = (uint8_t)(b > 255 ? 255 : b < 0 ? 0 : b);
			point[3] = UINT8_MAX;
		}
	}
}
