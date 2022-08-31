#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#include "Draw.h"
#include "Map.h"

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DLUGOSC_PLANSZY 2352

bool check_collision(SDL_Rect A, SDL_Rect B)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = A.x;
	rightA = A.x + A.w;
	topA = A.y;
	bottomA = A.y + A.h;

	//Calculate the sides of rect B
	leftB = B.x;
	rightB = B.x + B.w;
	topB = B.y;
	bottomB = B.y + B.h;

	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}
	//If none of the sides from A are outside B
	return true;
}

void koniec_gry(SDL_Surface* screen, int czarny, SDL_Texture* scrtex, SDL_Renderer* renderer, char *text, SDL_Surface* charset, Draw* test1, int &exit, int &quit, bool &odNowa, SDL_Event event)
{
	while (exit == 0) 
	{
		SDL_FillRect(screen, NULL, czarny);

		sprintf(text, "Czy chcesz kontynuowac y/n");
		(*test1).DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) 
				{
					quit = 1;
					exit = 1;
				}
				else if (event.key.keysym.sym == SDLK_n) 
				{
					quit = 1;
					exit = 1;
				}
				else if (event.key.keysym.sym == SDLK_y) 
				{
					odNowa = true;
					exit = 2;
				}
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) 
{
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed, gameTime;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti[2]; // animacja konia
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	Draw test1;
	Map* map;
	bool odNowa; // "n"
	int tryb; // "d"
	bool ifZ; // skok jesli mozliwy
	double const a = 0.2; // przyspieszenie = acceleration
	int Vp; // predkosc poczatkowa
	int jump, jump1, jump2; // skok
	double zTime; //jak dlugo trzymany przycik "z"
	bool reset; // zmienna do resetu wartosci na koniec (aby wystapil tylko raz)
	int falling; // zmienna regulujaca szybkosc spadania konia po skoku
	int zryw; //  zryw = 1 - brak zrywu | zryw = 15 - nastapil zryw
	bool canJump; // kon moze skoczyc
	SDL_Rect down1, up1, horsePos;
	int punkty, sekundy;
	bool ifZryw; // jesli zryw podczas zderzenia z gwiazda
	double xTime; // czas od ostatniego nacisniecia 'x'
	int horseX; // wspolrzedna x konia w przedziale 0-DLUGOSC_PLANSZY
	int index; // wyswietlanie animacji konia
	int exit;

	SDL_Rect wroPos, starPos;
	SDL_Surface* wro, *star, *bum, *dash;
	int random, random2; // losowa pozycja wrozki/gwiazdy
	int wroSpeed; 
	double wroDistance;
	int changeDirection; // zmiana pozycji wrozki
	bool display, display2;
	int starPoints, wroPoints;
	bool wroCol; // wrozka kolizja

	int zycia;
	SDL_Surface* zyc; // grafika zycia
	int next; // wspolrzedne do wyswietlania zyc

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) 
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) 
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Unicorn game");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// INICJUJ MOJ PROGRAM
	srand(time(NULL));

	map = new Map();

	//SDL_Rect up1;
	up1.x = 1020;
	up1.y = 140;
	up1.w = 30;
	up1.h = 80;

	//SDL_Rect down1;
	down1.x = 1300;
	down1.y = 335;
	down1.w = 75;
	down1.h = 75;

	// INICJUJ MOJ PROGRAM

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./images/cs8x8.bmp");
	if(charset == NULL) 
	{
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	eti[0] = SDL_LoadBMP("./images/kon2_0.bmp");
	if(eti[0] == NULL) 
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	eti[1] = SDL_LoadBMP("./images/kon2_1.bmp");
	if (eti[1] == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	dash = SDL_LoadBMP("./images/dash.bmp");
	if (dash == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	wro = SDL_LoadBMP("./images/wro2.bmp");
	if (wro == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	star = SDL_LoadBMP("./images/star.bmp");
	if (star == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	bum = SDL_LoadBMP("./images/bum.bmp");
	if (bum == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	zyc = SDL_LoadBMP("./images/zyc1.bmp");
	if (zyc == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	gameTime = 0;
	distance = 0;
	etiSpeed = 0;

	odNowa = false;

	tryb = 1;

	ifZ = 0;
	jump = 0;
	zTime = 0;
	jump1 = 0;
	jump2 = 0;
	canJump = 1;
	index = 0;

	zryw = 1;
	xTime = 0;

	falling = 0;

	punkty = 0;
	sekundy = 0;

	ifZryw = 0;

	horseX = 0;
	reset = 0;

	random = rand() % 1200;
	random2 = rand() % 600;
	wroSpeed = 1;
	wroDistance = 0;
	changeDirection = 0;
	display = true;
	display2 = true;
	starPoints = 100;
	wroPoints = 10;
	wroCol = 0;

	zycia = 3;
	next = 30;
	exit = 0;

	while(!quit) 
	{
		t2 = SDL_GetTicks();
		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		if (odNowa) 
		{
			distance = 0;
			gameTime = 0;
			punkty = 0;
			sekundy = 0;

			// wrozka i star
			starPoints = 100;
			wroPoints = 10;
			random = rand() % 1200;
			random2 = rand() % 600;
			display = true;
			display2 = true;
			wroCol = 0;

			// zycia
			if (zycia == 0) 
			{
				koniec_gry(screen, czarny, scrtex, renderer, text, charset, &test1, exit, quit, odNowa, event);
				if (exit == 2) 
				{
					zycia = 3;
				}
			}
			odNowa = false;
		}

		// czasy
		worldTime += delta;
		gameTime += delta;
		// jesli wscisniete 'Z'
		if (ifZ == 1) 
		{
			zTime += delta;
		}
		// zryw jesli wscisniete 'X'
		if (zryw != 1) 
		{
			xTime += delta;
		}

		// punkty za czas gry
		if (distance > sekundy) 
		{
			++punkty;
			sekundy += 50;
		}
		
		// animacja wrozki
		if (changeDirection != (int)gameTime)
		{
			changeDirection = (int)gameTime;
			if (changeDirection % 3 == 2)
			{
				wroSpeed = wroSpeed * (-1);
			}
		}

		// skok i spadek
		// spadek1
		if ((jump != 0 && ifZ == 0) || (jump != 0 && zTime >= 0.25))
		{
			++falling;
			if (falling == 3)
			{
				jump = jump - 1;
				falling = 0;
			}
		}

		// spadek2
		if (jump1 != 0 && jump == 0) 
		{
			jump = jump1;
			jump1 = 0;
		}

		// spadek3
		if (jump2 != 0 && jump1 == 0 && jump == 0)
		{
			jump = jump2;
			jump2 = 0;
		}

		// skok gdy 'Z'
		if (ifZ == 1 && zTime < 0.25) 
		{
			++jump;
		}

		// kon jest na ziemi
		if (jump2 == 0 && jump1 == 0 && jump == 0) 
		{
			canJump = 1;
		}

		// bieg konia
		if (tryb == 2) 
		{
			etiSpeed = Vp + a * gameTime; // V = V + a*t
		}

		wroDistance += wroSpeed * delta * 100; // dystans wrozki
		distance += zryw * etiSpeed * delta * 100; // dystans konia

		horseX = (int)distance%DLUGOSC_PLANSZY;

		SDL_FillRect(screen, NULL, czarny);

		map->DrawMap(screen, -horseX);

		// wyswietlanie zyc
		for (int i = 0; i < zycia; i++)
		{
			test1.DrawSurface(screen, zyc, next, 60);
			next = next + 50;
		}

		// rysowanie konia
		if (zryw == 1) 
		{
			test1.DrawSurface(screen, eti[index], 60, 290 - jump2 - jump1 - jump);
		}
		else 
		{
			test1.DrawSurface(screen, dash, 60, 290 - jump2 - jump1 - jump);
		}
		// rysowanie wrozki
		if (display == true) 
		{
			test1.DrawSurface(screen, wro, 500 - horseX + random + wroDistance, 220);
		}

		// rysowanie gwiazdy
		if (display2 == true) 
		{
			test1.DrawSurface(screen, star, 1400 - horseX + random2, 260);
		}
		else 
		{
			test1.DrawSurface(screen, bum, 1400 - horseX + random2, 260);
		}

		// pozycja konia
		// SDL_Rect horsePos;
		horsePos.x = horseX;
		horsePos.y = 330 - jump2 - jump1 - jump;
		horsePos.w = 121; 
		horsePos.h = 65; 

		// pozycja wrozki
		// SDL_Rect wroPos;
		wroPos.x = 470 + random + wroDistance;
		wroPos.y = 250;
		wroPos.w = 68;
		wroPos.h = 65;

		// pozycja gwiazdy
		// SDL_Rect starPos;
		starPos.x = 1370 + random2;
		starPos.y = 275;
		starPos.w = 85;
		starPos.h = 135;

		// kolizje
		if (check_collision(horsePos, up1) == true)
		{
			--zycia;
			odNowa = true;
		}
		else if (check_collision(horsePos, down1) == true)
		{
			--zycia;
			odNowa = true;
		}
		else if (check_collision(horsePos, starPos) == true && ifZryw == 0) 
		{
			if (zryw != 1) 
			{
				display2 = false;
				punkty = punkty + starPoints;
				starPoints = starPoints + 100;
				ifZryw = 1;
			}
			else 
			{
				--zycia;
				odNowa = true;
			}
		}
		else if (check_collision(horsePos, wroPos) == true && wroCol == 0)
		{
			punkty = punkty + wroPoints;
			wroPoints = wroPoints + 10;
			display = false;
			wroCol = 1; 
		}

		// reset wartosci
		// jesli nie w ruchu
		if (etiSpeed != 0) 
		{
			index = (index + 1) % 2;
		}

		exit = 0;
		if (xTime > 0.1) 
		{
			zryw = 1;
			xTime = 0;
		}
		next = 30;
		if (horseX > 2000 && reset == 0) 
		{
			// Star
			if (ifZryw == 0) 
			{
				starPoints = 100;
			}
			ifZryw = 0;
			reset = 1;
			random2 = rand() % 600;
			display2 = true;

			// wrozka
			random = rand() % 1200;
			display = true;
			if (wroCol == 0) 
			{
				wroPoints = 10;
			}
			wroCol = 0;
		}
		else if (horseX < 250) 
		{
			reset = 0;
		}
		// reset wartosci


		fpsTimer += delta;
		if(fpsTimer > 0.5) 
		{
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		// tekst informacyjny / info text
		test1.DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "time = %.1lf s,  %.0lf fps,  d-change mode, mode = %d", worldTime, fps, tryb);
		test1.DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//	      "Esc - exit, n - new game, x - super power, z - jump, d - change mode"
		sprintf(text, "esc-exit, n-new game, x-power, z-(2x)jump, \033 go right (if mode = 1)");
		test1.DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		sprintf(text, "czas = %.1lf", gameTime);
		test1.DrawString(screen, screen->w - 100 - strlen(text) * 8 / 2, 450, text, charset);

		sprintf(text, "%d", punkty);
		test1.DrawString(screen, screen->w - 325 - strlen(text) * 8 / 2, 50, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_RIGHT && tryb == 1) etiSpeed = 2.0;
					else if (event.key.keysym.sym == SDLK_z)
					{
						if (jump == 0) 
						{
							ifZ = 1;
						}
						if (jump != 0 && canJump == 1) 
						{
							canJump = 0;
							ifZ = 1;
							
							if (jump1 == 0) 
							{
								jump1 = jump;
								jump = 0;
							}
							else 
							{
								jump2 = jump1;
								jump1 = jump;
							}
						}
					}
					else if(event.key.keysym.sym == SDLK_n) odNowa = true;
					else if (event.key.keysym.sym == SDLK_d)
					{
						if (tryb == 1) 
						{
							etiSpeed = 1;
							Vp = 1;
							tryb = 2;
						}
						else if (tryb == 2) 
						{
							etiSpeed = 0;
							tryb = 1;
						}
					}
					else if (event.key.keysym.sym == SDLK_x)
					{
						if (zryw == 1) 
						{
							zryw = 15;
						}
						canJump = 1;
					}
					break;
				case SDL_KEYUP:
					if (tryb == 1) 
					{
						etiSpeed = 0;
					}
					zTime = 0;
					ifZ = 0;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
			};
		};
		frames++;
	};

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};