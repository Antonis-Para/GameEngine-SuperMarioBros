#include "Color.h"

void SetPalette(RGB* palette) {

}

Color Make8(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color Make16(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color Make24(RGBValue r, RGBValue g, RGBValue b) {
	return al_map_rgb(r, g, b);
}

Color Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha = 0) {
	return al_map_rgba(r, g, b, alpha);
}