#ifndef COLOR
#define COLOR

#include <allegro5/allegro.h>

typedef ALLEGRO_COLOR Color;
typedef unsigned char RGBValue;
typedef unsigned char Alpha;
struct RGB { RGBValue r, g, b; };
struct RGBA : public RGB { RGBValue a; };
typedef RGB Palette[256];
extern void SetPalette(RGB* palette);
extern Color Make8(RGBValue r, RGBValue g, RGBValue b);
extern Color Make16(RGBValue r, RGBValue g, RGBValue b);
extern Color Make24(RGBValue r, RGBValue g, RGBValue b);
extern Color Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha = 0);

#endif