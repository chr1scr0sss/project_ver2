#pragma once
#include <string>
#include <vector>
#include "SDL2/SDL.h"



class TextureLoader
{
public:
	static SDL_Texture* loadTexture(std::string filename, SDL_Renderer* renderer);
	static void deallocateTextures();


private:
	static std::vector<SDL_Texture*> listTexturesLoaded;
};