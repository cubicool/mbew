#include "mbew.h"

#include <SDL/SDL.h>

int main(int argc, char** argv) {
	mbew_t* mbew = mbew_create(MBEW_SRC_FILE, argv[1]);
	mbew_status_t status;

	if(!(status = mbew_status(mbew))) {
		mbew_iter_t* iter = NULL;
		mbew_num_t width = mbew_property(mbew, MBEW_PROP_VIDEO_WIDTH).num;
		mbew_num_t height = mbew_property(mbew, MBEW_PROP_VIDEO_HEIGHT).num;
		mbew_num_t start;

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

		start = SDL_GetTicks();

		printf("Starting at: %ums\n", start);

		while(mbew_iterate(mbew, &iter, 0)) {
			SDL_Event event;

			if(mbew_iter_type(iter) == MBEW_DATA_VIDEO) {
				mbew_data_video_t* video = mbew_iter_video(iter);
				mbew_num_t now = SDL_GetTicks() - start;
				mbew_num_t timestamp = mbew_iter_timestamp(iter) / 1000000;
				mbew_num_t y;

				printf(
					"Index=%u Time(now)=%ums Time(frame)=%ums ",
					mbew_iter_index(iter),
					now,
					timestamp
				);

				if(now < timestamp) {
					printf("[sleeping for: %ums]", timestamp - now);

					SDL_Delay(timestamp - now);
				}

				printf("\n");

				SDL_LockYUVOverlay(overlay);

				for(y = 0; y < video->height; ++y) memcpy(
					overlay->pixels[0] + (overlay->pitches[0] * y),
					video->data.yuv.planes[0] + (video->data.yuv.stride[0] * y),
					overlay->pitches[0]
				);

				for(y = 0; y < video->height >> 1; ++y) memcpy(
					overlay->pixels[1] + (overlay->pitches[1] * y),
					video->data.yuv.planes[2] + (video->data.yuv.stride[2] * y),
					overlay->pitches[1]
				);

				for(y = 0; y < video->height >> 1; ++y) memcpy(
					overlay->pixels[2] + (overlay->pitches[2] * y),
					video->data.yuv.planes[1] + (video->data.yuv.stride[1] * y),
					overlay->pitches[2]
				);

				SDL_UnlockYUVOverlay(overlay);
				SDL_DisplayYUVOverlay(overlay, &rect);
			}

			if(SDL_PollEvent(&event) == 1) {
				if(event.type == SDL_KEYDOWN) {
					if(event.key.keysym.sym == SDLK_ESCAPE) break;
					if(event.key.keysym.sym == SDLK_SPACE) SDL_WM_ToggleFullScreen(surface);
				}
			}
		}

		mbew_iter_destroy(iter);

		SDL_FreeSurface(surface);
		SDL_Quit();
	}

	else printf("Error creating context (%s)\n", mbew_string(MBEW_TYPE_STATUS, status));

	mbew_destroy(mbew);

	return 0;
}

