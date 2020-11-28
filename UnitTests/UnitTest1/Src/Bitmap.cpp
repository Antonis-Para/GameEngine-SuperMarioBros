#include "Bitmap.h"

Bitmap BitmapLoad(const std::string& path) {
	return al_load_bitmap(path.c_str());
}

Bitmap BitmapCreate(unsigned short w, unsigned short h){
	return al_create_bitmap(w, h);
}

Bitmap BitmapCopy(Bitmap bmp) {
	return al_clone_bitmap(bmp);
}

Bitmap BitmapClear(Bitmap bmp, Color c) {
	al_set_target_bitmap(bmp);
	al_clear_to_color(Make24(0, 0, 0));
	return bmp;
}

void BitmapDestroy(Bitmap bmp) {
	al_destroy_bitmap(bmp);
}

Bitmap BitmapGetScreen(void) {
	return NULL;
}

unsigned short BitmapGetWidth(Bitmap bmp) {
	return al_get_bitmap_width(bmp);
}

unsigned short BitmapGetHeight(Bitmap bmp) {
	return al_get_bitmap_height(bmp);
}

void BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to) {
	al_set_target_bitmap(dest);
	al_draw_bitmap(src, to.x, to.y, 0);
}