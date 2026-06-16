#include "mbew.h"

#include <SDL3/SDL.h>
#include <stdio.h>

int main(int argc, char** argv) {
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		mbew_num_t width = mbew_property(m, MBEW_PROPERTY_VIDEO_WIDTH).num;
		mbew_num_t height = mbew_property(m, MBEW_PROPERTY_VIDEO_HEIGHT).num;

		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Texture* texture;
		Uint64 start;
		int running = 1;

		SDL_Init(SDL_INIT_VIDEO);

		window = SDL_CreateWindow("mbew-example-video-sdl", width, height, 0);
		renderer = SDL_CreateRenderer(window, NULL);

		/* mbew_iter_yuv_planes() yields planes in [Y, U, V] order, which matches
		 * SDL_PIXELFORMAT_IYUV directly--no plane swap needed. */
		texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_IYUV,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height
		);

		SDL_SetRenderVSync(renderer, 1);

		start = SDL_GetTicksNS();

		while(running) {
			SDL_Event event;

			while(SDL_PollEvent(&event)) {
				if(event.type == SDL_EVENT_QUIT) running = 0;

				if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) running = 0;
			}

			if(mbew_iterate(m, MBEW_ITERATE_VIDEO | MBEW_ITERATE_SYNC)) {
				if(mbew_iter_sync(m, SDL_GetTicksNS() - start)) {
					mbew_bytes_t* planes = mbew_iter_yuv_planes(m);
					mbew_num_t* stride = mbew_iter_yuv_stride(m);

					SDL_UpdateYUVTexture(
						texture,
						NULL,
						planes[0],
						stride[0],
						planes[1],
						stride[1],
						planes[2],
						stride[2]
					);
				}
			}

			else {
				printf("Finished a single playthrough; resetting.\n");

				if(!mbew_reset(m)) break;

				start = SDL_GetTicksNS();
			}

			SDL_RenderClear(renderer);
			SDL_RenderTexture(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}
