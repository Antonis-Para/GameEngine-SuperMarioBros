#include "Grid.h"

using namespace app;

static GridMap grid;
static TileColorsHolder emptyTileColors;

void SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index) {
	(*m)[row][col] = index;
}

GridIndex GetGridTile(const GridMap* m, Dim col, Dim row) {
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
	return (GetGridTile(m, row, col) & flags) != 0;
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
			for (auto row = startRow; row <= endRow; ++row)
				if (!CanPassGridTile(m, newCol, row, GRID_RIGHT_SOLID_MASK)) {
					*dx = MUL_GRID_ELEMENT_WIDTH(currCol) - r.x;
					break;
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
				if (!CanPassGridTile(m, newRow, col, GRID_BOTTOM_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_HEIGHT(newRow) - (y2 + 1);
					break;
				}
		}
	}
}

void FilterGridMotionDown(GridMap* m, const Rect& r, int* dy) {
	auto y1_next = r.y + *dy;
	if (y1_next < 0)
		*dy = -r.y;
	else {
		auto newRow = DIV_GRID_ELEMENT_HEIGHT(y1_next);
		auto currRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
		if (newRow != currRow) {
			assert(newRow + 1 == currRow); // we really move left
			auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
			for (auto col = startCol; col <= endCol; ++col)
				if (!CanPassGridTile(m, newRow, col, GRID_TOP_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_WIDTH(currRow) - r.y;
					break;
				}
		}
	}
}

void ComputeTileGridBlocks1(const TileMap* map, GridIndex* grid) {
	for (auto row = 0; row < MAX_HEIGHT; ++row)
		for (auto col = 0; col < MAX_WIDTH; ++col) {
			memset(grid, IsTileIndexAssumedEmpty(GetTile(map, col, row)) ? GRID_EMPTY_TILE : GRID_SOLID_TILE, GRID_ELEMENTS_PER_TILE);
			grid += GRID_ELEMENTS_PER_TILE;
		}
}

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
		auto isEmpty = ComputeIsGridIndexEmpty(tmp2, transColor, solidThreshold);
		*grid++ = isEmpty ? GRID_EMPTY_TILE : GRID_SOLID_TILE;
	}
}

bool ComputeIsGridIndexEmpty(Bitmap gridElement, Color transColor, unsigned char solidThreshold) {
	auto n = 0;
	BitmapAccessPixels(gridElement, [transColor, &n](unsigned char* mem) {
		auto c = GetPixel32(mem);
		if (c != transColor && !IsTileColorEmpty(c))
			++n;
		});
	return n <= solidThreshold;
}


void TileColorsHolder::Insert(Bitmap bmp, Index index) {
	if (indices.find(index) == indices.end()) {
		indices.insert(index);
		BitmapAccessPixels(bmp, [this](unsigned char* mem) {
			colors.insert(GetPixel32(mem));
			});
	}
}

bool TileColorsHolder::In(Color c) const {
	return colors.find(c) != colors.end();
}