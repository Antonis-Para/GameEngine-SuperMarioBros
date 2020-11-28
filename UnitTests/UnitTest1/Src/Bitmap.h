#ifndef BITMAP
#define BITMAP

#include <string>
#include <allegro5/allegro.h>
#include "Structs.h"

typedef ALLEGRO_BITMAP* Bitmap;
Bitmap BitmapLoad(const std::string& path);
Bitmap BitmapCreate(unsigned short w, unsigned short h);
Bitmap BitmapCopy(Bitmap bmp);
//Bitmap BitmapClear(Bitmap bmp, Color c);
void BitmapDestroy(Bitmap bmp);
Bitmap BitmapGetScreen(void);
unsigned short BitmapGetWidth(Bitmap bmp);
unsigned short BitmapGetHeight(Bitmap bmp);
void BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to);

#endif