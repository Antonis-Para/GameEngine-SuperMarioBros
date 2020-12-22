#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include "Defines.h"

//--------------------TYPEDEFS--------------------------
typedef unsigned short Dim;
typedef unsigned short Index;
typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
typedef ALLEGRO_BITMAP* Bitmap;
typedef ALLEGRO_COLOR Color;
typedef unsigned char RGBValue;
typedef unsigned char Alpha;
typedef unsigned char* PixelMemory;


//grid
using GridIndex = unsigned char;
typedef GridIndex GridMap[GRID_MAX_HEIGHT][GRID_MAX_WIDTH];
typedef unsigned char* PixelMemory;