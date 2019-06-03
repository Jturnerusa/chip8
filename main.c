#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "SDL.h"
#include "cpu.h"

void handle_input(struct cpu * const c8, const Uint8 *keys) {
	if (keys[SDL_SCANCODE_1]) set_key(c8, 0);
	if (keys[SDL_SCANCODE_2]) set_key(c8, 1);
	if (keys[SDL_SCANCODE_3]) set_key(c8, 2);
	if (keys[SDL_SCANCODE_4]) set_key(c8, 3);
	
	if (keys[SDL_SCANCODE_Q]) set_key(c8, 4);
	if (keys[SDL_SCANCODE_W]) set_key(c8, 5);
	if (keys[SDL_SCANCODE_E]) set_key(c8, 6);
	if (keys[SDL_SCANCODE_R]) set_key(c8, 7);
	
	if (keys[SDL_SCANCODE_A]) set_key(c8, 8);
	if (keys[SDL_SCANCODE_S]) set_key(c8, 9);
	if (keys[SDL_SCANCODE_D]) set_key(c8, 10);
	if (keys[SDL_SCANCODE_F]) set_key(c8, 11);
	
	if (keys[SDL_SCANCODE_Z]) set_key(c8, 12);
	if (keys[SDL_SCANCODE_X]) set_key(c8, 13);
	if (keys[SDL_SCANCODE_C]) set_key(c8, 14);
	if (keys[SDL_SCANCODE_V]) set_key(c8, 15);
}

void draw_pixels(struct cpu * const c8, SDL_Renderer *renderer) {
	int x, y;
	int w = get_gfx_w(c8);
	int h = get_gfx_h(c8);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //White
	for (x = 0; x < w; x++) {
		for (y = 0; y < h; y++) {
			if (get_pixel(c8, x, y)) {
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); //Black
	SDL_RenderClear(renderer);
}

int main(int argc, char* args[]) {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;
	const Uint8 *keys;
	FILE *rom;
	struct cpu *c8;
	/*
	*The resolution can be whatever, but non-multiples of 64 and 32 for the
	*width and height can look weird due to scaling issues.
	*/
	int screen_w = 64*20;
	int screen_h = 32*20;
	if (argc < 2) {
		printf("Usage: CHIP-8 MyRomFile.rom");
		exit(EXIT_FAILURE);
	}
	rom = fopen(args[1], "rb");
		if (rom == NULL) {
			printf("Failed to open ROM.");
			exit(EXIT_FAILURE);
		}
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr,
				"\nUnable to Init SDL: %s\n",
				SDL_GetError()
				);
	}
	SDL_CreateWindowAndRenderer(screen_w, screen_h, 0, &window, &renderer);
	SDL_SetWindowTitle(window, "CHIP-8");
	keys = SDL_GetKeyboardState(NULL);
	while (1) {
		c8 = init_cpu(rom);
		SDL_RenderSetScale(renderer, 
						   screen_w / get_gfx_w(c8), 
						   screen_h / get_gfx_h(c8)
						   );
		while (1) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					SDL_Quit();
					exit(1);
				}
			}
			if (keys[SDL_SCANCODE_ESCAPE]) {
				break;
			}
			handle_input(c8, keys);
			emulate_cycle(c8);
			draw_pixels(c8, renderer);
		}
		//Free old cpu.
		dealloc_cpu(c8);
		rewind(rom);
	}
}