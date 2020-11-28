#include <iostream>
#include "app.h"

int main() {
	app::TileMap m;
	app::ReadTextMap(&m, "D:\\mraptakis\\Documents\\CSD\\Εξάμηνο 7ο\\ΗΥ-454\\Super_Mario\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");

	for (int i = 0; i < 840; ++i)
		for (int j = 0; j < 42; ++i)
			std::cout << m[i][j] << " " << std::endl;

	return EXIT_SUCCESS;
}