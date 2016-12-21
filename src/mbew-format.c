#include "mbew-private.h"

/* http://stackoverflow.com/questions/29327705/libvpx-convert-vpx-img-fmt-i420-rgb */
void mbew_format_rgb(vpx_image_t* img, uint8_t* dest) {
	mbew_num_t width = img->d_w;
	mbew_num_t height = img->d_h;
	const uint8_t* y = img->planes[VPX_PLANE_Y];
	const uint8_t* u = img->planes[VPX_PLANE_U];
	const uint8_t* v = img->planes[VPX_PLANE_V];
	mbew_num_t ystride = img->stride[VPX_PLANE_Y];
	mbew_num_t ustride = img->stride[VPX_PLANE_U];
	mbew_num_t vstride = img->stride[VPX_PLANE_V];

    unsigned long int i;
	unsigned long int j;

    for(i = 0; i < height; ++i) {
        for(j = 0; j < width; ++j) {
			int r;
			int g;
			int b;

            uint8_t* point = dest + 4 * ((i * width) + j);

            int t_y = y[((i * ystride) + j)];
            int t_u = u[(((i / 2) * ustride) + (j / 2))];
            int t_v = v[(((i / 2) * vstride) + (j / 2))];

            t_y = t_y < 16 ? 16 : t_y;

            r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;

            point[2] = r > 255 ? 255 : r < 0 ? 0 : r;
            point[1] = g > 255 ? 255 : g < 0 ? 0 : g;
            point[0] = b > 255 ? 255 : b < 0 ? 0 : b;
            point[3] = ~0;
        }
    }
}

