#include "Grid.h"

using namespace app;

GridMap grid;
static TileColorsHolder emptyTileColors;

void SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index) {
	(*m)[row][col] = index;
}

GridIndex GetGridTile(const GridMap* m, Dim col, Dim row) {
	//return(*m)[MUL_GRID_ELEMENT_HEIGHT(row)][MUL_GRID_ELEMENT_WIDTH(col)];
	return(*m)[row][col];
}

void SetSolidGridTile(GridMap* m, Dim col, Dim row) {
	SetGridTile(m, col, row, GRID_SOLID_TILE);
}

void SetEmptyGridTile(GridMap* m, Dim col, Dim row) {
	SetGridTile(m, col, row, GRID_EMPTY_TILE);
}

void SetGridTileFlags(GridMap* m, Dim col, Dim row, GridIndex flags) {
	SetGridTile(m, col, row, flags);
}

void SetGridTileTopSolidOnly(GridMap* m, Dim col, Dim row) {
	SetGridTileFlags(m, row, col, GRID_TOP_SOLID_MASK);
}

bool CanPassGridTile(GridMap* m, Dim col, Dim row, GridIndex flags) {
	//cout << (int)GetGridTile(m, row, col) << endl;
	return (GetGridTile(m, col, row) & flags) == 0;
}

void FilterGridMotion(GridMap* m, const Rect& r, int* dx, int* dy) {
	assert(abs(*dx) <= GRID_ELEMENT_WIDTH && abs(*dy) <= GRID_ELEMENT_HEIGHT);

	// try horizontal move
	if (*dx < 0)
		FilterGridMotionLeft(m, r, dx);
	else if (*dx > 0)
		FilterGridMotionRight(m, r, dx);

	// try vertical move
	if (*dy < 0)
		FilterGridMotionUp(m, r, dy);
	else if (*dy > 0)
		FilterGridMotionDown(m, r, dy);
}

void FilterGridMotionLeft(GridMap* m, const Rect& r, int* dx) {
	auto x1_next = r.x + *dx;
	if (x1_next < 0)
		*dx = -r.x;
	else {
		auto newCol = DIV_GRID_ELEMENT_WIDTH(x1_next);
		auto currCol = DIV_GRID_ELEMENT_WIDTH(r.x);
		if (newCol != currCol) {
			assert(newCol + 1 == currCol); // we really move left
			auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
			auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y + r.h - 1);
			for (auto row = startRow; row <= endRow; ++row) {
				if (!CanPassGridTile(m, newCol, row, GRID_RIGHT_SOLID_MASK)) {
					*dx = MUL_GRID_ELEMENT_WIDTH(currCol) - r.x;
					break;
				}
			}
		}
	}
}

void FilterGridMotionRight(GridMap* m, const Rect& r, int* dx) {
	auto x2 = r.x + r.w - 1;
	auto x2_next = x2 + *dx;
	if (x2_next >= MAX_PIXEL_WIDTH)
		*dx = MAX_PIXEL_WIDTH - x2;
	else {
		auto newCol = DIV_GRID_ELEMENT_WIDTH(x2_next);
		auto currCol = DIV_GRID_ELEMENT_WIDTH(x2);
		if (newCol != currCol) {
			assert(newCol - 1 == currCol); // we really move right
			auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
			auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y + r.h - 1);
			for (auto row = startRow; row <= endRow; ++row)
				if (!CanPassGridTile(m, newCol, row, GRID_LEFT_SOLID_MASK)) {
					*dx = MUL_GRID_ELEMENT_WIDTH(newCol) - (x2 + 1);
					break;
				}
		}
	}
}

void FilterGridMotionUp(GridMap* m, const Rect& r, int* dy) {
	auto y1_next = r.y + *dy;
	if (y1_next < 0)
		*dy = -r.y;
	else {
		auto newRow = DIV_GRID_ELEMENT_HEIGHT(y1_next);
		auto currRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
		if (newRow != currRow) {
			assert(newRow + 1 == currRow); // we really move up
			auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
			for (auto col = startCol; col <= endCol; ++col)
				if (!CanPassGridTile(m, col, newRow, GRID_TOP_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_WIDTH(currRow) - r.y;
					break;
				}
		}
	}
}

void FilterGridMotionDown(GridMap* m, const Rect& r, int* dy) {
	auto y2 = r.y + r.h - 1;
	auto y2_next = y2 + *dy;
	if (y2_next >= MAX_PIXEL_HEIGHT)
		*dy = MAX_PIXEL_HEIGHT - y2;
	else {
		auto newRow = DIV_GRID_ELEMENT_HEIGHT(y2_next);
		auto currRow = DIV_GRID_ELEMENT_HEIGHT(y2);
		if (newRow != currRow) {
			assert(newRow - 1 == currRow); // we really move down
			auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
			for (auto col = startCol; col <= endCol; ++col)
				if (!CanPassGridTile(m, col, newRow, GRID_BOTTOM_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_HEIGHT(newRow) - (y2 + 1);
					break;
				}
		}
	}
}

bool IsTileIndexAssumedEmpty(app::Index index) {
	switch (index) {
		case 48:
		case 49:
		case 50:
		case 54:
		case 55:
		case 56:
		case 60:
		case 62:
		case 66:
		case 68:
		case 72:
		case 73:
		case 74:
		case 78:
		case 79:
		case 80:
		case 85:
		case 84:
		case 61:
		case 96:
		case 97:
			return false;
			break;
	}
	return true;
}

void ComputeTileGridBlocks1(const TileMap* map, GridIndex* grid) {
	for (auto row = 0; row < MAX_HEIGHT; ++row) {
		GridIndex* tmp2 = grid;
		for (auto col = 0; col < MAX_WIDTH; ++col) {
			GridIndex* tmp = grid;
			for (auto k = 0; k < GRID_ELEMENT_HEIGHT; ++k) {
				memset(grid, IsTileIndexAssumedEmpty(GetTile(map, row, col)) ? GRID_EMPTY_TILE : GRID_SOLID_TILE, GRID_ELEMENT_WIDTH);
				grid += GRID_MAX_WIDTH;
			}
			grid = tmp + GRID_ELEMENT_WIDTH;
			//memset(grid, IsTileIndexAssumedEmpty(GetTile(map, row, col)) ? GRID_EMPTY_TILE : GRID_SOLID_TILE, GRID_ELEMENTS_PER_TILE);
			//grid += GRID_ELEMENTS_PER_TILE;
		}
		//grid += GRID_MAX_WIDTH * 3;
		grid = tmp2 + GRID_MAX_WIDTH * 4;
	}
}
/*
bool IsTileColorEmpty(Color c) {
	return emptyTileColors.In(c);
}

void ComputeTileGridBlocks2(const TileMap* map, GridIndex* grid, Bitmap tileSet, Color transColor, unsigned char solidThreshold) {
	auto tileElem = BitmapCreate(TILE_WIDTH, TILE_HEIGHT);
	auto gridElem = BitmapCreate(GRID_ELEMENT_WIDTH, GRID_ELEMENT_HEIGHT);
	for (auto row = 0; row < MAX_HEIGHT; ++row)
		for (auto col = 0; col < MAX_WIDTH; ++col) {
			auto index = GetTile(map, col, row);
			PutTile(tileElem, 0, 0, tileSet, index);
			if (IsTileIndexAssumedEmpty(index)) {
				emptyTileColors.Insert(tileElem, index);// assume tile colors to be empty
				memset(grid, GRID_EMPTY_TILE, GRID_ELEMENTS_PER_TILE);
				grid += GRID_ELEMENTS_PER_TILE;
			}
			else
				ComputeGridBlock(grid, index, tileElem, gridElem, tileSet, transColor, solidThreshold);
		}
	BitmapDestroy(tileElem);
	BitmapDestroy(gridElem);
}

void ComputeGridBlock(GridIndex*& grid, Index index, Bitmap tileElem, Bitmap gridElem, Bitmap tileSet, Color transColor, unsigned char solidThreshold) {
	for (auto i = 0; i < GRID_ELEMENTS_PER_TILE; ++i) {
		auto x = i % GRID_BLOCK_ROWS;
		auto y = i / GRID_BLOCK_ROWS;
		BitmapBlit(tileElem, { x * GRID_ELEMENT_WIDTH, y * GRID_ELEMENT_HEIGHT, GRID_ELEMENT_WIDTH, GRID_ELEMENT_HEIGHT }, gridElem, { 0, 0 });
		auto isEmpty = ComputeIsGridIndexEmpty(tileSet, transColor, solidThreshold);
		*grid++ = isEmpty ? GRID_EMPTY_TILE : GRID_SOLID_TILE;
	}
}

bool ComputeIsGridIndexEmpty(Bitmap gridElement, Color transColor, unsigned char solidThreshold) {
	auto n = 0;
	BitmapAccessPixels(gridElement, [transColor, &n](unsigned char* mem) {
		auto c = GetPixel32(mem);
		if (c.r != transColor.r && c.g != transColor.g && c.b != transColor.b && !IsTileColorEmpty(c))
			++n;
		});
	return n <= solidThreshold;
}*/