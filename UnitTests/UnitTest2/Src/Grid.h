#pragma once
#include "app.h"

#define GRID_ELEMENT_WIDTH 4
#define GRID_ELEMENT_HEIGHT 4
#if TILE_WIDTH % GRID_ELEMENT_WIDTH != 0
#error "TILE_WIDTH % GRID_ELEMENT_WIDTH must be zero!"
#endif
#if TILE_HEIGHT % GRID_ELEMENT_HEIGHT != 0
#error "TILE_HEIGHT % GRID_ELEMENT_HEIGHT must be zero!"
#endif
#define GRID_BLOCK_COLUMNS (TILE_WIDTH / GRID_ELEMENT_WIDTH)
#define GRID_BLOCK_ROWS (TILE_HEIGHT / GRID_ELEMENT_HEIGHT)
#define GRID_ELEMENTS_PER_TILE (GRID_BLOCK_ROWS * GRID_BLOCK_COLUMNS)
#define GRID_MAX_HEIGHT (MAX_HEIGHT * GRID_BLOCK_ROWS)
#define GRID_MAX_WIDTH (MAX_WIDTH * GRID_BLOCK_COLUMNS)

#define GRID_THIN_AIR_MASK 0x0000 // element is ignored
#define GRID_LEFT_SOLID_MASK 0x0001 // bit 0
#define GRID_RIGHT_SOLID_MASK 0x0002 // bit 1
#define GRID_TOP_SOLID_MASK 0x0004 // bit 2
#define GRID_BOTTOM_SOLID_MASK 0x0008 // bit 3
#define GRID_GROUND_MASK 0x0010 // bit 4, keep objects top / bottom (gravity)
#define GRID_FLOATING_MASK 0x0020 // bit 5, keep objects anywhere inside (gravity)
#define GRID_EMPTY_TILE GRID_THIN_AIR_MASK
#define GRID_SOLID_TILE \
		(GRID_LEFT_SOLID_MASK | GRID_RIGHT_SOLID_MASK | GRID_TOP_SOLID_MASK | GRID_BOTTOM_SOLID_MASK)

#define MAX_PIXEL_WIDTH MUL_TILE_WIDTH(MAX_WIDTH)
#define MAX_PIXEL_HEIGHT MUL_TILE_HEIGHT(MAX_HEIGHT)
#define DIV_GRID_ELEMENT_WIDTH(i) ((i) >> 2)
#define DIV_GRID_ELEMENT_HEIGHT(i) ((i) >> 2)
#define MUL_GRID_ELEMENT_WIDTH(i) ((i) << 2)
#define MUL_GRID_ELEMENT_HEIGHT(i) ((i) << 2)

using GridIndex = unsigned char;
typedef GridIndex GridMap[GRID_MAX_WIDTH][GRID_MAX_HEIGHT];

void SetGridTile(GridMap* m, app::Dim col, app::Dim row, GridIndex index);

GridIndex GetGridTile(const GridMap* m, app::Dim col, app::Dim row);

void SetSolidGridTile(GridMap* m, app::Dim col, app::Dim row);

void SetEmptyGridTile(GridMap* m, app::Dim col, app::Dim row);

void SetGridTileFlags(GridMap* m, app::Dim col, app::Dim row, GridIndex flags);

void SetGridTileTopSolidOnly(GridMap* m, app::Dim col, app::Dim row);

bool CanPassGridTile(GridMap* m, app::Dim col, app::Dim row, GridIndex flags);

void FilterGridMotion(GridMap* m, const app::Rect& r, int* dx, int* dy);

void FilterGridMotionLeft(GridMap* m, const app::Rect& r, int* dx);

void FilterGridMotionRight(GridMap* m, const app::Rect& r, int* dx);

void FilterGridMotionUp(GridMap* m, const app::Rect& r, int* dx);

void FilterGridMotionDown(GridMap* m, const app::Rect& r, int* dx);



extern bool IsTileIndexAssumedEmpty(app::Index index);
void ComputeTileGridBlocks1(const app::TileMap* map, GridIndex* grid);

bool IsTileColorEmpty(app::Color c);

void ComputeTileGridBlocks2(const app::TileMap* map, GridIndex* grid, app::Bitmap tileSet, app::Color transColor, unsigned char solidThreshold);

void ComputeGridBlock(GridIndex*& grid, app::Index index, app::Bitmap tileElem, app::Bitmap gridElem, app::Bitmap tileSet, app::Color transColor, unsigned char solidThreshold);

bool ComputeIsGridIndexEmpty(app::Bitmap gridElement, app::Color transColor, unsigned char solidThreshold);