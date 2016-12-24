#include "mbew.h"

#include <SDL/SDL.h>

/* Return MBEW_TRUE if the video plays through in its entirety and MBEW_FALSE if ESC is pressed to
 * interrupt the application. */
mbew_bool_t play_video(mbew_t m, SDL_Overlay* overlay, SDL_Rect* rect) {
	mbew_num_t start = SDL_GetTicks();

	printf("Starting at: %ums\n", start);

	while(mbew_iterate(m, MBEW_ITER_VIDEO_ONLY)) {
		SDL_Event event;

		mbew_num_t now = SDL_GetTicks() - start;
		mbew_num_t timestamp = mbew_iter_timestamp(m) / 1000000;
		mbew_bytes_t* planes = mbew_iter_video_yuv_planes(m);
		mbew_num_t* stride = mbew_iter_video_yuv_stride(m);
		mbew_num_t y;

		printf(
			"Index=%u Time(now)=%ums Time(frame)=%ums ",
			mbew_iter_index(m),
			now,
			timestamp
		);

		if(now < timestamp) {
			printf("[sleeping for: %ums]", timestamp - now);

			SDL_Delay(timestamp - now);
		}

		printf("\n");

		SDL_LockYUVOverlay(overlay);

		for(y = 0; y < rect->h; ++y) memcpy(
			overlay->pixels[0] + (overlay->pitches[0] * y),
			planes[0] + (stride[0] * y),
			overlay->pitches[0]
		);

		for(y = 0; y < (rect->h >> 1); ++y) memcpy(
			overlay->pixels[1] + (overlay->pitches[1] * y),
			planes[2] + (stride[2] * y),
			overlay->pitches[1]
		);

		for(y = 0; y < (rect->h >> 1); ++y) memcpy(
			overlay->pixels[2] + (overlay->pitches[2] * y),
			planes[1] + (stride[1] * y),
			overlay->pitches[2]
		);

		SDL_UnlockYUVOverlay(overlay);
		SDL_DisplayYUVOverlay(overlay, rect);

		if(SDL_PollEvent(&event) == 1) {
			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return MBEW_FALSE;
		}
	}

	return MBEW_TRUE;
}

int main(int argc, char** argv) {
	mbew_t m = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(m))) {
		mbew_num_t width = mbew_property(m, MBEW_PROP_VIDEO_WIDTH).num;
		mbew_num_t height = mbew_property(m, MBEW_PROP_VIDEO_HEIGHT).num;

		SDL_Surface* surface = NULL;
		SDL_Overlay* overlay = NULL;
		SDL_Rect rect;

		SDL_Init(SDL_INIT_VIDEO);

		surface = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
		overlay = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, surface);

		rect.x = 0;
		rect.y = 0;
		rect.w = width;
		rect.h = height;

		while(play_video(m, overlay, &rect)) {
			printf("Finished a single playthrough; resetting.\n");

			if(!mbew_reset(m)) {
				printf("Couldn't reset: %s\n", mbew_string(MBEW_TYPE_STATUS, mbew_status(m)));

				break;
			}
		}

		SDL_FreeYUVOverlay(overlay);
		SDL_FreeSurface(surface);
		SDL_Quit();
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(m);

	return 0;
}

