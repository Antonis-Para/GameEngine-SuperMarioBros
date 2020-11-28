#include "app.h"

using namespace app;

void SetPalette(RGB* palette) {

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