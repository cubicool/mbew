#include "mbew.h"

#include <SDL/SDL.h>

void fill_audio(void* userdata, Uint8* stream, int ssize) {
	mbew_t m = (mbew_t)(userdata);

	if(mbew_iterate(m, MBEW_ITERATE_AUDIO)) {
		const int16_t* data = mbew_iter_pcm16(m);
		int size = (int)(mbew_iter_pcm16_size(m));

		if(!size) return;

		SDL_MixAudio(stream, (Uint8*)(data), ssize, SDL_MIX_MAXVOLUME);
	}
}

int main(int argc, char** argv) {
	mbew_t m;

	if(argc < 2) {
		printf("Must specify WebM file.\n");

		return 1;
	}

	m = mbew_create(MBEW_SOURCE_FILE, argv[1]);

	if(mbew_valid(m)) {
		int play = 1;

		SDL_AudioSpec spec = {
			.freq = mbew_property(m, MBEW_PROPERTY_AUDIO_RATE).hz,
			.format = AUDIO_S16,
			.channels = mbew_property(m, MBEW_PROPERTY_AUDIO_CHANNELS).num,
			.samples = 1024,
			.callback = fill_audio,
			.userdata = m
		};

		SDL_Init(SDL_INIT_AUDIO);
		SDL_OpenAudio(&spec, NULL);

		/* ... */
		mbew_iterate(m, 0);

		SDL_PauseAudio(MBEW_FALSE);

		while(play) {
			SDL_Delay(100);
			SDL_LockAudio();

			if(!mbew_iter_active(m)) play = 0;

			SDL_UnlockAudio();
		}

		SDL_PauseAudio(MBEW_TRUE);
		SDL_CloseAudio();
		SDL_Quit();
	}

	else printf("Error creating context (%s)\n", mbew_string(mbew_status(m)));

	mbew_destroy(m);

	return 0;
}

