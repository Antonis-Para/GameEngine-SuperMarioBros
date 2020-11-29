#include "app.h"

using namespace app;

Bitmap app::BitmapLoad(const std::string& path) {
	return al_load_bitmap(path.c_str());
}

Bitmap app::BitmapCreate(unsigned short w, unsigned short h){
	return al_create_bitmap(w, h);
}

Bitmap app::BitmapCopy(Bitmap bmp) {
	return al_clone_bitmap(bmp);
}

Bitmap app::BitmapClear(Bitmap bmp, Color c) {
	al_set_target_bitmap(bmp);
	al_clear_to_color(Make24(0, 0, 0));
	return bmp;
}

void app::BitmapDestroy(Bitmap bmp) {
	al_destroy_bitmap(bmp);
}

Bitmap app::BitmapGetScreen(void) {
	return NULL;
}

unsigned short app::BitmapGetWidth(Bitmap bmp) {
	return al_get_bitmap_width(bmp);
}

unsigned short app::BitmapGetHeight(Bitmap bmp) {
	return al_get_bitmap_height(bmp);
}

void app::BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to) {
	Bitmap tile = al_create_sub_bitmap(src, from.x, from.y, from.w, from.h);
	al_set_target_bitmap(dest);
	al_draw_bitmap(tile, to.x, to.y, 0);
}