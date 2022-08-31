#pragma once

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 805
#define SCREEN_HEIGHT 718


class Map
{
public:

	Map();
	//~Map();

	//void LoadMap(int arr[3][5]);
	void DrawMap(SDL_Surface* screen, int move);

private:
	//SDL_Rect src, dest;
	int x, y, a, b;

	//SDL_Surface* screen;
	SDL_Surface* grass;
	SDL_Surface* water;
	//SDL_Surface* pavement;

	//int mep[3][5];
};
