#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_events.h>

const int WIDTH = 320;
const int HEIGHT = 240;

const int BULLETCACHE = 20;

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

	w = SDL_CreateWindow("space shooty", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
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
	uint32_t nextShoot = 0, nextMove = 0;
	uint32_t shootTime = 1000 * 0.2, moveTime = 1000 * 0.1;

	SDL_Rect bulletSrc = { .x = 16 * 13, .y = 16 * 2, .w = 16, .h = 16 };
	SDL_Rect bullets[BULLETCACHE];
	int i, j;
	for (i = 0; i < BULLETCACHE; i++) {
		bullets[i].x = -1;
	}

	SDL_Rect enemySrc = { .x = 16, .y = 16, .w = 16, .h = 16};
	SDL_Rect enemies[20];
	for (i = 0; i < 20; i++) {
		enemies[i].x = WIDTH + 16;
		enemies[i].w = 16;
		enemies[i].h = 16;
	}
	uint32_t enemySpawnTime = 1000 * 3, nextEnemy = 0;
	uint32_t enemyMoveTime = 1000 * 1, nextEnemyMove = 0;
	uint32_t enemySpawns = 2;
	uint32_t enemyCount = enemySpawns;

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
				if (!up && !down && SDL_GetTicks() >= nextMove)
					nextMove = SDL_GetTicks() + moveTime;
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						up = 1;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						down = 1;
						break;
					case SDL_SCANCODE_SPACE:
						if (SDL_GetTicks() >= nextShoot) {
							for (i = 0; i < BULLETCACHE; i++) {
								if (bullets[i].x == -1) {
									bullets[i] = playerDest;
									break;
								}
							}
							nextShoot = SDL_GetTicks() + shootTime;
						}
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

		vel = 0;
		if ((SDL_GetTicks() >= nextMove) && (up || down)) {
			if ((up && !down) && playerDest.y > 0) vel = -16;
			if ((down && !up) && playerDest.y <= HEIGHT - playerDest.h * 2) vel = 16;
			playerDest.y += vel;
			nextMove = SDL_GetTicks() + moveTime;
		}

		SDL_RenderClear(r);
		SDL_SetTextureColorMod(t, 255, 255, 255);
		SDL_RenderCopy(r, t, &playerSrc, &playerDest);

		if (SDL_GetTicks() >= nextEnemy) {
			if (enemyCount > 0) {
				for (i = 0; i < 20; i++) {
					if (enemies[i].x == WIDTH + 16) {
						enemies[i].x -= 16;
						enemies[i].y = (rand() % (HEIGHT / 16 + 1)) * 16;
						break;
					}
				}
				--enemyCount;
				nextEnemy = SDL_GetTicks() + enemySpawnTime;
			} else {
				enemySpawns *= 2;
				enemyCount = enemySpawns;
				nextEnemy = SDL_GetTicks() + enemySpawnTime * 2;
			}
		}

		SDL_SetTextureColorMod(t, 255, 255, 0);
		for (i = 0; i < BULLETCACHE; i++) {
			if (bullets[i].x != -1) {
				if (bullets[i].x < WIDTH) {
					bullets[i].x += 16;
					for (j = 0; j < 20; j++) {
						if (bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
							enemies[j].x = WIDTH + 16;
							bullets[i].x = -1;;
							continue;
						}
					}
					if (bullets[i].x != -1)
						SDL_RenderCopy(r, t, &bulletSrc, &bullets[i]);
				} else {
					bullets[i].x = -1;
				}
			}
		}

		SDL_SetTextureColorMod(t, 255, 0, 0);
		for (i = 0; i < 20; i++) {
			if (enemies[i].x <= WIDTH) {
				if (SDL_GetTicks() >= nextEnemyMove) {
					if (enemies[i].x == 0) {
						enemies[i].x = WIDTH + 16;
					} else {
						enemies[i].x -= 16;
					}
				}
				SDL_RenderCopy(r, t, &enemySrc, &enemies[i]);
			}
		}
		if (SDL_GetTicks() >= nextEnemyMove) {
			nextEnemyMove = SDL_GetTicks() + enemyMoveTime;
		}

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
