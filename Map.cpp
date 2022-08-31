#include "Map.h"
#include "Draw.h"

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

/*
int map[3][5] =
{
	{ 0,0,0,0,0 },
	{ 2,2,2,2,2 },
	{ 2,2,2,2,2 }
}; 
*/
Map::Map()
{
	grass = SDL_LoadBMP("images/platform8.bmp");
	water = SDL_LoadBMP("images/tlo3.bmp");
	//pavement = SDL_LoadBMP("pavement.bmp");

	//LoadMap(map);
}

/*Map::~Map() 
{
	delete *mep;
	delete grass;
	delete pavement;
	delete water;
	delete *x;
	delete y;
	delete a;
	delete b;
}*/

/*
void Map::LoadMap(int arr[3][5])
{
	for (int wiersz = 0; wiersz < 3; wiersz++) 
	{
		for (int kolumna = 0; kolumna < 5; kolumna++) 
		{	
			mep[wiersz][kolumna] = arr[wiersz][kolumna];
		}
	}
}
*/
void Map::DrawMap(SDL_Surface* screen, int move)
{
	Draw test;
	test.DrawSurface(screen, water, 1600+move, 300);
	test.DrawSurface(screen, grass, 1600+move, 200);
}
