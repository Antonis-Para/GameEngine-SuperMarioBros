#include "allegro5/allegro.h"

int main() {
	al_init();

	al_create_display(640, 480);

	al_clear_to_color(al_map_rgb(255, 0, 255));

	al_flip_display();

	al_rest(5.0f);

	return EXIT_SUCCESS;
}