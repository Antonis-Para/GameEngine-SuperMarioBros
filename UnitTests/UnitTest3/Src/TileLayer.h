#pragma once
#include "Typedefs.h"
#include "Defines.h"
#include <string>
#include "GridLayer.h"

class GridLayer;

class TileLayer {
	private:
		Index* map = nullptr;
		GridLayer* grid = nullptr;
		Dim totalRows = 0, totalColumns = 0;
		Bitmap tileSet = nullptr;
		Rect viewWin;
		Bitmap dpyBuffer = nullptr;
		bool dpyChanged = true;
		Dim dpyX = 0, dpyY = 0;


		/*Pre caching*/
		Index* divIndex = nullptr;
		Index* modIndex = nullptr;
		Dim TileXc(Index index);
		Dim TileYc(Index index);

		void FilterScrollDistance(
			int viewStartCoord,// x  or y
			int viewSize,// w  or h
			int* d,// dx or dy
			int maxMapSize// w or h
		);
		void FilterScroll(int* dx, int* dy);
		void PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile);
	public:
		void Allocate(void);
		void InitCaching(int width, int height);

		void SetTile(Dim col, Dim row, Index index);
		Index GetTile(Dim col, Dim row) const;
		void ComputeTileGridBlocks1(void);
		GridLayer* GetGrid(void) const;
		const Point Pick(Dim x, Dim y) const;

		const Rect& GetViewWindow(void) const;
		void SetViewWindow(const Rect& r);

		void TileTerrainDisplay(Bitmap dest, const Rect& displayArea, Bitmap tiles);
		Bitmap GetBitmap(void) const;
		int GetPixelWidth(void) const;
		int GetPixelHeight(void) const;
		unsigned GetTileWidth(void) const;
		unsigned GetTileHeight(void) const;

		void Scroll(int dx, int dy);
		bool CanScrollHoriz(int dx) const;
		bool CanScrollVert(int dy) const;
		void ScrollWithBoundsCheck(int dx, int dy);
		void ScrollWithBoundsCheck(int *dx, int *dy);

		auto ToString(void) const -> const std::string; // unparse
		bool FromString(const std::string&); // parse
		void Save(const std::string& path) const;
		 
		bool Load(const std::string& path);
		FILE* WriteText(FILE* fp) const;
		bool ReadText(FILE* fp); // TODO: carefull generic parsing
		TileLayer(Dim _rows, Dim _cols);
		~TileLayer(); // cleanup here with care!
};


//class GridLayer {
//	private:
//		GridIndex* grid = nullptr;
//		unsigned total = 0;
//		Dim totalRows = 0, totalColumns = 0;
//		void Allocate(void);
//		// TODO: adapt as needed and insert all rest motion control functions
//		// inside the private section
//		void FilterGridMotionDown(const Rect& r, int* dy) const;
//		void FilterGridMotionLeft(const Rect& r, int* dy) const;
//		void FilterGridMotionRight(const Rect& r, int* dy) const;
//		void FilterGridMotionUp(const Rect& r, int* dy) const;
//	public:
//		void FilterGridMotion(const Rect& r, int* dx, int* dy) const;
//		bool IsOnSolidGround(const Rect& r) const;
//		GridIndex*& GetBuffer(void);
//		const GridIndex* GetBuffer(void) const;
//		GridLayer(unsigned rows, unsigned cols);
//};