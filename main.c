#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_events.h>

const int WIDTH = 160;
const int HEIGHT = 160;

int main() {
	SDL_Window *w;
	SDL_Renderer *r;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
		SDL_Log("init error; %s", SDL_GetError());
		return 1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		SDL_Log("error initializing SDL_image; %s", IMG_GetError());
		return 1;
	}

	w = SDL_CreateWindow("space shooty", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	if (!w) {
		SDL_Log("error creating window; %s", SDL_GetError());
		SDL_Quit();
		IMG_Quit();
		return 1;
	}
	r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
	if (!r) {
		SDL_DestroyWindow(w);
		SDL_Log("error creating renderer; %s", SDL_GetError());
		SDL_Quit();
		IMG_Quit();
		return 1;
	}

	SDL_Surface *s = IMG_Load("sprites.png");
	if (!s) {
		SDL_DestroyWindow(w);
		SDL_DestroyRenderer(r);
		SDL_Log("error loading sprites; %s", IMG_GetError());
		SDL_Quit();
		IMG_Quit();
		return 1;
	}
	IMG_Quit();
	if (SDL_SetColorKey(s, SDL_TRUE, SDL_MapRGB(s->format, 255, 0, 255)) != 0) {
		SDL_DestroyWindow(w);
		SDL_DestroyRenderer(r);
		SDL_Log("error setting sprite color key; %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
	if (!t) {
		SDL_DestroyWindow(w);
		SDL_DestroyRenderer(r);
		SDL_Log("error loading sprites as texture; %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_FreeSurface(s);

	SDL_Rect playerSrc = { .x = 16 * 4, .y = 16 * 14, .w = 16, .h = 16 };
	SDL_Rect playerDest = { .x = 0, .y = 0, .w = 16, .h = 16 };
	int up = 0, down = 0, vel = 0;

	SDL_Event e;
	int q = 0;
	uint32_t start, now;
	while (!q) {
		start = SDL_GetTicks();

		while (SDL_PollEvent(&e))
		switch (e.type) {
			case SDL_QUIT:
				q = 1;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						up = 1;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						down = 1;
						break;
				}
				break;
			case SDL_KEYUP:
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						up = 0;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						down = 0;
						break;
				}
		}

		SDL_RenderClear(r);
		SDL_RenderCopy(r, t, &playerSrc, &playerDest);
		SDL_RenderPresent(r);
		if ((now = SDL_GetTicks())- start < 1000)
			SDL_Delay((1000 - (now - start)) / 60);
	}

	SDL_DestroyWindow(w);
	SDL_DestroyRenderer(r);
	SDL_DestroyTexture(t);
	SDL_Quit();
	return 0;
}
