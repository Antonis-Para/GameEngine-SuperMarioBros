#include "app.h"

int main() {
	app::TileMap m;
	app::ReadTextMap(&m, "C:\\Users\\george\\Desktop\\MagicLand.csv");

	return EXIT_SUCCESS;
}