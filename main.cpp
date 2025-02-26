#include <iostream>
#include "SDL2/SDL.h"

#include "Game.h"



int main(int argc, char* args[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Error: Couldn't initialize SDL Video = " << SDL_GetError() << std::endl;
		return 1;
	}
	else {
		//Create the window.
		const int boardSize = 1024;  //define size window game
		SDL_Window* window = SDL_CreateWindow("Checkers", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, boardSize, boardSize, 0);

		if (window == nullptr) {
			std::cout << "Error: Couldn't create window = " << SDL_GetError() << std::endl;
			return 1;
		}
		else {
			//Create a renderer for GPU accelerated drawing.
			SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == nullptr) {
				std::cout << "Error: Couldn't create renderer = " << SDL_GetError() << std::endl;
				return 1;
			}
			else {
				//Ensure transparent graphics are drawn correctly.
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

				//Enable anti-aliasing to make the game look better if the window is scaled.
				SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

				//Output the name of the render driver.
				SDL_RendererInfo rendererInfo;
				SDL_GetRendererInfo(renderer, &rendererInfo);
				std::cout << "Renderer = " << rendererInfo.name << std::endl;

				//Start the game.
				Game game(window, renderer, boardSize);

				//Clean up.
				SDL_DestroyRenderer(renderer);
			}

			//Clean up.
			SDL_DestroyWindow(window);
		}

		//Clean up.
		SDL_Quit();
	}


	return 0;
}