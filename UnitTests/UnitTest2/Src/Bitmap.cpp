#include "app.h"
#include <allegro5/allegro.h>

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

int app::BitmapGetWidth(Bitmap bmp) {
	return al_get_bitmap_width(bmp);
}

int app::BitmapGetHeight(Bitmap bmp) {
	return al_get_bitmap_height(bmp);
}

void app::BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to) {
	Bitmap tile = al_create_sub_bitmap(src, from.x, from.y, from.w, from.h);
	al_set_target_bitmap(dest);
	al_draw_bitmap(tile, to.x, to.y, 0);
	al_destroy_bitmap(tile);
}

ALLEGRO_LOCKED_REGION* app::BitmapLock(Bitmap bmp) {
	return al_lock_bitmap(bmp, al_get_bitmap_format(bmp), ALLEGRO_LOCK_READWRITE);
}

void app::BitmapUnlock(Bitmap bmp) {
	al_unlock_bitmap(bmp);
}

//TODO
PixelMemory app::BitmapGetMemory(Bitmap bmp) {
	//return bmp;
}

int app::BitmapGetLineOffset(Bitmap bmp) {
	//is this ok?
	return al_get_bitmap_width(bmp);
	//return 1;
}

//TODO
int GetDepth(Bitmap bmp) {
	//return get_color_depth(); 
	//return al_get_bitmap_depth(bmp);
	return 1;
}

template<typename Tfunc>
void app::BitmapAccessPixels(Bitmap bmp, const Tfunc& f) {
	auto result = BitmapLock(bmp);
	assert(result);
	auto mem = BitmapGetMemory(bmp);
	auto offset = BitmapGetLineOffset(bmp);
	for (auto y = BitmapGetHeight(bmp); y--;) {
		auto buff = mem;
		for (auto x = BitmapGetWidth(bmp); x--;) {
			f(buff);
			buff += GetDepth();
		}
		mem += offset;
	}
	
	BitmapUnlock(bmp);
}