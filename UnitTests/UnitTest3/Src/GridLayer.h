#pragma once
//#include "app.h"
#include "Defines.h"
#include "Typedefs.h"
#include "TileLayer.h"

class GridLayer {
private:
	GridMap grid;
	unsigned total = 0;
	Dim totalRows = 0, totalColumns = 0;
	void Allocate(void);

	void FilterGridMotionDown(const Rect& r, int* dy);
	void FilterGridMotionLeft(const Rect& r, int* dy);
	void FilterGridMotionRight(const Rect& r, int* dy);
	void FilterGridMotionUp(const Rect& r, int* dy);

	bool IsTileIndexAssumedEmpty(Index index);
	bool IsTileColorEmpty(Color c);
	bool ComputeIsGridIndexEmpty(Bitmap gridElement, Color transColor, unsigned char solidThreshold);
public:
	GridLayer();
	void FilterGridMotion(const Rect& r, int* dx, int* dy);
	bool IsOnSolidGround(const Rect& r);
	GridMap* GetBuffer(void);
	const GridMap* GetBuffer(void) const;
	//GridLayer(unsigned rows, unsigned cols);
	void ComputeTileGridBlocks1(class TileLayer *tilelayer);
	void ComputeTileGridBlocks2(const TileMap* map, GridIndex* grid, Bitmap tileSet, Color transColor, unsigned char solidThreshold);
	void ComputeGridBlock(GridIndex*& grid, Index index, Bitmap tileElem, Bitmap gridElem, Bitmap tileSet, Color transColor, unsigned char solidThreshold);
};

void SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index);

GridIndex GetGridTile(const GridMap* m, Dim col, Dim row);

void SetSolidGridTile(GridMap* m, Dim col, Dim row);

void SetEmptyGridTile(GridMap* m, Dim col, Dim row);

void SetGridTileFlags(GridMap* m, Dim col, Dim row, GridIndex flags);

void SetGridTileTopSolidOnly(GridMap* m, Dim col, Dim row);

bool CanPassGridTile(GridMap* m, Dim col, Dim row, GridIndex flags);