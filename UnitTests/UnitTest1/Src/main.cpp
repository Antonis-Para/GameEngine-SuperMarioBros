#include "app.h"

int main() {
	app::TileMap m;
	app::ReadTextMap(&m, ".\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");

	for (int i = 0; i < 42; ++i) {
		for (int j = 0; j < 21; ++j)
			std::cout << m[i][j] << " ";
		std::cout << std::endl;
	}

	ALLEGRO_DISPLAY* display = NULL;
	// Initialize allegro
	if (!al_init()) {
		fprintf(stderr, "Failed to initialize allegro.\n");
		return 1;
	}

	// Create the display
	display = al_create_display(640, 480);
	if (!display) {
		fprintf(stderr, "Failed to create display.\n");
		return 1;
	}

	al_destroy_display(display);

	return EXIT_SUCCESS;
}