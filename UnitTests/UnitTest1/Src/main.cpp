#include "app.h"

int main() {
	app::TileMap m;
	app::ReadTextMap(&m, "D:\\mraptakis\\Documents\\CSD\\Εξάμηνο 7ο\\ΗΥ-454\\Super_Mario\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");

	for (int i = 0; i < 42; ++i) {
		for (int j = 0; j < 21; ++j)
			std::cout << m[i][j] << " ";
		std::cout << std::endl;
	}

	return EXIT_SUCCESS;
}