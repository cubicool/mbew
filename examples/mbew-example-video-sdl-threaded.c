#include "mbew.h"

#include <SDL3/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Number of fully-decoded frames the decode thread is allowed to stay ahead of the render
 * thread. Bounds memory to RING_CAPACITY preallocated frame buffers, no matter how long the
 * source video is. */
#define RING_CAPACITY 3

typedef struct {
	mbew_bytes_t data;
	mbew_ns_t timestamp;
} frame_slot_t;

/* A single-producer/single-consumer ring buffer: the decode thread is the only writer, the
 * render thread (main()) is the only reader. */
typedef struct {
	frame_slot_t slots[RING_CAPACITY];
	mbew_num_t write;
	mbew_num_t read;
	mbew_num_t count;
	mbew_bool_t done;
	mbew_bool_t stop;

	pthread_mutex_t lock;
	pthread_cond_t not_empty;
	pthread_cond_t not_full;
} frame_ring_t;

typedef struct {
	mbew_t m;
	frame_ring_t* ring;
	mbew_num_t width;
	mbew_num_t height;
} decode_args_t;

void frame_ring_init(frame_ring_t* ring, mbew_num_t frame_size) {
	mbew_num_t i;

	memset(ring, 0, sizeof(frame_ring_t));

	for(i = 0; i < RING_CAPACITY; i++) ring->slots[i].data = (mbew_bytes_t)(malloc(frame_size));

	pthread_mutex_init(&ring->lock, NULL);
	pthread_cond_init(&ring->not_empty, NULL);
	pthread_cond_init(&ring->not_full, NULL);
}

void frame_ring_destroy(frame_ring_t* ring) {
	mbew_num_t i;

	for(i = 0; i < RING_CAPACITY; i++) free(ring->slots[i].data);

	pthread_mutex_destroy(&ring->lock);
	pthread_cond_destroy(&ring->not_empty);
	pthread_cond_destroy(&ring->not_full);
}

/* mbew_iter_yuv_planes()/yuv_stride() point into libvpx's own internal buffer pool, which gets
 * reused on the next mbew_iterate() call--so the frame must be fully copied out, row by row
 * respecting the source stride, before the decode thread moves on. */
void frame_copy_yuv(
	mbew_bytes_t dest,
	mbew_bytes_t* planes,
	mbew_num_t* stride,
	mbew_num_t width,
	mbew_num_t height
) {
	mbew_num_t cw = (width + 1) / 2;
	mbew_num_t ch = (height + 1) / 2;
	mbew_num_t y;

	mbew_bytes_t dest_u = dest + (width * height);
	mbew_bytes_t dest_v = dest_u + (cw * ch);

	for(y = 0; y < height; y++) memcpy(dest + (y * width), planes[0] + (y * stride[0]), width);
	for(y = 0; y < ch; y++) memcpy(dest_u + (y * cw), planes[1] + (y * stride[1]), cw);
	for(y = 0; y < ch; y++) memcpy(dest_v + (y * cw), planes[2] + (y * stride[2]), cw);
}

void* decode_thread(void* arg) {
	decode_args_t* args = (decode_args_t*)(arg);
	frame_ring_t* ring = args->ring;
	mbew_ns_t loop_offset = 0;
	mbew_bool_t stop = MBEW_FALSE;

	while(!stop) {
		while(mbew_iterate(args->m, MBEW_ITERATE_VIDEO)) {
			mbew_bytes_t* planes = mbew_iter_yuv_planes(args->m);
			mbew_num_t* stride = mbew_iter_yuv_stride(args->m);

			pthread_mutex_lock(&ring->lock);

			while(ring->count == RING_CAPACITY && !ring->stop) {
				pthread_cond_wait(&ring->not_full, &ring->lock);
			}

			if(ring->stop) {
				stop = MBEW_TRUE;

				pthread_mutex_unlock(&ring->lock);

				break;
			}

			frame_copy_yuv(ring->slots[ring->write].data, planes, stride, args->width, args->height);

			ring->slots[ring->write].timestamp = loop_offset + mbew_iter_timestamp(args->m);
			ring->write = (ring->write + 1) % RING_CAPACITY;
			ring->count++;

			pthread_cond_signal(&ring->not_empty);
			pthread_mutex_unlock(&ring->lock);
		}

		if(stop) break;

		printf("Finished a single playthrough; resetting.\n");

		/* Keep timestamps monotonically increasing across playthroughs so the render thread's
		 * single, fixed pacing clock keeps working unmodified--without this, every playthrough
		 * after the first would blow past its pacing check instantly and speed through. */
		loop_offset += mbew_property(args->m, MBEW_PROPERTY_DURATION).ns;

		if(!mbew_reset(args->m)) break;
	}

	pthread_mutex_lock(&ring->lock);

	ring->done = MBEW_TRUE;

	pthread_cond_signal(&ring->not_empty);
	pthread_mutex_unlock(&ring->lock);

	return NULL;
}

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
		mbew_num_t frame_size = mbew_video_frame_size(m, 0);
		mbew_num_t cw = (width + 1) / 2;
		mbew_num_t ch = (height + 1) / 2;

		frame_ring_t ring;
		decode_args_t args = { .m = m, .ring = &ring, .width = width, .height = height };
		pthread_t thread;

		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Texture* texture;
		Uint64 start;
		int running = 1;

		frame_ring_init(&ring, frame_size);

		SDL_Init(SDL_INIT_VIDEO);

		window = SDL_CreateWindow("mbew-example-video-sdl-threaded", width, height, 0);
		renderer = SDL_CreateRenderer(window, NULL);

		texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_IYUV,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height
		);

		SDL_SetRenderVSync(renderer, 1);

		pthread_create(&thread, NULL, decode_thread, &args);

		start = SDL_GetTicksNS();

		while(running) {
			SDL_Event event;

			while(SDL_PollEvent(&event)) {
				if(event.type == SDL_EVENT_QUIT) running = 0;

				if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) running = 0;
			}

			pthread_mutex_lock(&ring.lock);

			while(!ring.count && !ring.done) pthread_cond_wait(&ring.not_empty, &ring.lock);

			if(ring.count && SDL_GetTicksNS() - start >= ring.slots[ring.read].timestamp) {
				mbew_bytes_t data = ring.slots[ring.read].data;

				SDL_UpdateYUVTexture(
					texture,
					NULL,
					data,
					width,
					data + (width * height),
					cw,
					data + (width * height) + (cw * ch),
					cw
				);

				ring.read = (ring.read + 1) % RING_CAPACITY;
				ring.count--;

				pthread_cond_signal(&ring.not_full);
			}

			else if(!ring.count && ring.done) running = 0;

			pthread_mutex_unlock(&ring.lock);

			SDL_RenderClear(renderer);
			SDL_RenderTexture(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		/* If we're exiting early (ESC/window close) rather than because the decode thread
		 * finished on its own, tell it to stop so pthread_join() below doesn't wait on a
		 * thread that's still blocked filling a ring nobody is draining anymore. */
		pthread_mutex_lock(&ring.lock);
		ring.stop = MBEW_TRUE;
		pthread_cond_signal(&ring.not_full);
		pthread_mutex_unlock(&ring.lock);

		pthread_join(thread, NULL);

		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();

		frame_ring_destroy(&ring);
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}
