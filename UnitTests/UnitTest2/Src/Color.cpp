#include "app.h"

using namespace app;

void app::SetPalette(RGB* palette) {

}

Color app::Make8(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color app::Make16(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color app::Make24(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color app::Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha = 0) {
	return al_map_rgba(r, g, b, alpha);
}

void app::ReadPixelColor32(void* mem, RGBA *c, Alpha *a) {
	Color cl = al_get_pixel((Bitmap)mem, 0, 0);
	al_unmap_rgba(cl, &c->r, &c->g, &c->b, a);
}

Color app::GetPixel32(void* mem) {
	RGBA c;
	ReadPixelColor32(mem, &c, &c.a);
	return Make32(c.r, c.g, c.b, c.a);
}