#pragma once
#include "app.h"
#include "Defines.h"

using GridIndex = unsigned char;
typedef GridIndex GridMap[GRID_MAX_HEIGHT][GRID_MAX_WIDTH];

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

bool IsTileIndexAssumedEmpty(app::Index index);

void ComputeTileGridBlocks1(const app::TileMap* map, GridIndex* grid);

bool IsTileColorEmpty(app::Color c);

void ComputeTileGridBlocks2(const app::TileMap* map, GridIndex* grid, app::Bitmap tileSet, app::Color transColor, unsigned char solidThreshold);

void ComputeGridBlock(GridIndex*& grid, app::Index index, app::Bitmap tileElem, app::Bitmap gridElem, app::Bitmap tileSet, app::Color transColor, unsigned char solidThreshold);

bool ComputeIsGridIndexEmpty(app::Bitmap gridElement, app::Color transColor, unsigned char solidThreshold);