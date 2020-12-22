#include "TileLayer.h"
#include "Bitmap.h"

//Rect viewWin = app::Rect{ 0, 0, VIEW_WIN_X, VIEW_WIN_Y };
//Rect displayArea = app::Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };

TileLayer::TileLayer() {
	//do nothing
}

TileLayer::~TileLayer() {
	//Do nothing for now
}

extern Rect displayArea;

void TileLayer::Allocate(void) {
	//map = new Index[totalRows * totalColumns];
	dpyBuffer = BitmapCreate(displayArea.w + TILE_WIDTH, displayArea.h + TILE_HEIGHT);
	viewWin = Rect{ 0, 0, VIEW_WIN_X, VIEW_WIN_Y };
}

void TileLayer::AllocateCaching(int width, int height) {
	int size = width * height;
	divIndex = new Index[size];
	modIndex = new Index[size];

	for (int i = 0; i < size; ++i) {
		divIndex[i] = MUL_TILE_HEIGHT(i / width); //y
		modIndex[i] = MUL_TILE_WIDTH(i % width); //x
	}
}

const Point TileLayer::Pick(Dim x, Dim y) const
{
	return { DIV_TILE_WIDTH(x + viewWin.x),
	DIV_TILE_HEIGHT(y + viewWin.y) };
}

const Rect& TileLayer::GetViewWindow(void) const { 
	return viewWin; 
}

void TileLayer::SetViewWindow(const Rect& r)
{
	viewWin = r;
	dpyChanged = true;
}


Bitmap TileLayer::GetBitmap(void) const { return dpyBuffer; }

int TileLayer::GetPixelWidth(void) const { return viewWin.w; }

int TileLayer::GetPixelHeight(void) const { return viewWin.h; }

unsigned TileLayer::GetTileWidth(void) const { return DIV_TILE_WIDTH(viewWin.w); }

unsigned TileLayer::GetTileHeight(void) const { return DIV_TILE_HEIGHT(viewWin.h); }

void TileLayer::Save(const std::string& path) const
{
	//fclose(WriteText(fopen(path.c_str(), "wt")));
}

FILE* TileLayer::WriteText(FILE* fp) const
{
	//fprintf(fp, "%s", TileLayer::ToString().c_str());
	return fp;
}

void TileLayer::SetTile(Dim col, Dim row, Index index) {
	map[row][col] = index;
}

Index TileLayer::GetTile(Dim row, Dim col) const{
	return map[row][col];
}

bool TileLayer::CanScrollHoriz(int dx) const{
	return viewWin.x >= -dx && (viewWin.x + viewWin.w + dx) <= GetMapPixelWidth();
}

bool TileLayer::CanScrollVert(int dy) const{
	return viewWin.y >= -dy && (viewWin.y + viewWin.h + dy) <= GetMapPixelHeight();
}

void TileLayer::Scroll(int dx, int dy) {
	viewWin.x += dx;
	viewWin.y += dy;
	dpyChanged = true;
}

void TileLayer::FilterScrollDistance(
	int viewStartCoord,// x  or y
	int viewSize,// w  or h
	int* d,// dx or dy
	int maxMapSize// w or h
) {
	auto val = *d + viewStartCoord;
	if (val < 0)
		*d = viewStartCoord;
	else if ((val + viewSize) >= maxMapSize)
		*d = maxMapSize - (viewStartCoord + viewSize);
}

void TileLayer::FilterScroll(int* dx, int* dy) {
	FilterScrollDistance(viewWin.x, viewWin.w, dx, GetMapPixelWidth());
	FilterScrollDistance(viewWin.y, viewWin.h, dy, GetMapPixelHeight());
}

void TileLayer::ScrollWithBoundsCheck(int dx, int dy) {
	FilterScroll(&dx, &dy);
	Scroll(dx, dy);
}

void TileLayer::ScrollWithBoundsCheck(int* dx, int* dy) {
	FilterScroll(dx, dy);
	Scroll(*dx, *dy);
}

Dim TileLayer::TileXc(Index index) {
	return modIndex[index];
}

Dim TileLayer::TileYc(Index index) {
	return divIndex[index];
}

void TileLayer::PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile) {
	BitmapBlit(tiles, Rect{ TileXc(tile), TileYc(tile), TILE_WIDTH, TILE_HEIGHT }, dest, Point{ x, y });
}

void TileLayer::TileTerrainDisplay(Bitmap dest, const Rect& displayArea, Bitmap tiles) {
	if (dpyChanged) {
		auto startCol = DIV_TILE_WIDTH(viewWin.x);
		auto startRow = DIV_TILE_HEIGHT(viewWin.y);
		auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
		auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.h - 1);
		dpyX = MOD_TILE_WIDTH(viewWin.x);
		dpyY = MOD_TILE_HEIGHT(viewWin.y);
		dpyChanged = false;
		for (Dim row = startRow; row <= endRow; ++row) {
			for (Dim col = startCol; col <= endCol; ++col) {
				PutTile(dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(row, col));
			}
		}
	}

	BitmapBlit(dpyBuffer, { dpyX, dpyY, viewWin.w, viewWin.h }, dest, { displayArea.x, displayArea.y });
}


//-----------GRID LAYER-----------------

//void GridLayer::Allocate(void) {
//	grid = new GridIndex[total = totalRows * totalColumns];
//	memset(grid, GRID_EMPTY_TILE, total);
//}
//
//bool GridLayer::IsOnSolidGround(const Rect& r) const { // will need later for gravity
//	int dy = 1; // down 1 pixel
//	FilterGridMotionDown(r, &dy);
//	return dy == 0; // if true IS attached to solid ground
//}
//
//GridIndex*& GridLayer::GetBuffer(void) { return grid; }
//
//const GridIndex* GridLayer::GetBuffer(void) const { return grid; }