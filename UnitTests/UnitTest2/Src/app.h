#pragma once

#ifndef APP_H
#define APP_H

#include <functional>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

using namespace std;

namespace app {
	//extern Bitmap tiles;


	//--------------------DEFINES---------------------------
	/*size of each tile in pixels*/
	#define TILE_WIDTH 16
	#define TILE_HEIGHT 16

	#define VIEW_WIN_X 640
	#define VIEW_WIN_Y 480
	#define DISP_AREA_X 640
	#define DISP_AREA_Y 480

	#define MUL_TILE_WIDTH(i) ((i) << 4)
	#define MUL_TILE_HEIGHT(i) ((i) << 4)
	#define DIV_TILE_WIDTH(i) ((i) >> 4)
	#define DIV_TILE_HEIGHT(i) ((i) >> 4)
	#define MOD_TILE_WIDTH(i) ((i) & 15)
	#define MOD_TILE_HEIGHT(i) ((i) & 15)
	#define MAX_WIDTH 1024
	#define MAX_HEIGHT 256
	#define TILEX_MASK 0xFF00
	#define TILEX_SHIFT 8
	#define TILEY_MASK 0x00FF

	#define MAX_VIEWS 4

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

	//--------------------TYPEDEFS--------------------------
	typedef unsigned short Dim;
	typedef unsigned short Index;
	typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
	typedef ALLEGRO_BITMAP* Bitmap;
	typedef ALLEGRO_COLOR Color;
	typedef unsigned char RGBValue;
	typedef unsigned char Alpha;

	using BitmapAccessFunctor = std::function<void(unsigned char**)>;

	using GridIndex = unsigned char;
	typedef GridIndex GridMap[GRID_MAX_WIDTH][GRID_MAX_HEIGHT];

	//--------------------STRUCTS---------------------------
	struct Rect {
		int x, y, w, h;
	};
	struct Point {
		int x, y;
	};
	struct RGB { 
		RGBValue r, g, b; 
	};
	struct RGBA : public RGB { 
		RGBValue a; 
	};

	struct ViewData {
		Bitmap dpyBuffer = nullptr;
		bool dpyChanged = true;
		Dim dpyX = 0, dpyY = 0;
		Rect viewWin = app::Rect{ 0, 0, VIEW_WIN_X, VIEW_WIN_Y };
		Rect displayArea = app::Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
	};

	//--------------------TYPEDEFS 2------------------------
	typedef RGB Palette[256];

	//--------------------CLASSES---------------------------
	class Game {
	public:
		using Action = std::function<void(void)>;
		using Pred = std::function<bool(void)>;
	private:
		Action render, anim, input, ai, physics, destruct, collisions, user;
		Pred done;

		void Invoke(const Action& f);
	public:
		template <typename Tfunc> void SetDone(const Tfunc& f);
		template <typename Tfunc> void SetRender(const Tfunc& f);
		template <typename Tfunc> void SetInput(const Tfunc& f);
		// rest of setters are similary defined
		void Render(void);
		void ProgressAnimations(void);
		void Input(void);
		void AI(void);
		void Physics(void);
		void CollisionChecking(void);
		void CommitDestructions(void);
		void UserCode(void);
		bool IsFinished(void) const;
		void MainLoop(void);
		void MainLoopIteration(void);
	};


	class App {
		protected:
			Game game;

		public:
			virtual void		Initialise (void) = 0;
			virtual void		Load (void) = 0;
			virtual void		Run(void);
			virtual void		RunIteration(void);
			Game&				GetGame(void);
			const Game&			GetGame(void) const;
			virtual void		Clear (void) = 0;
			void Main(void);
	};

	class MainApp : public App {
		public:
			void	Initialise(void);
			void	Load(void);
			void	Clear(void);
	};

	class TileColorsHolder final {
		private:
			std::set<Index> indices;
			std::set<Color> colors;
		public:
			void Insert(Bitmap bmp, Index index);
			bool In(Color c) const;
	};

	//--------------------FUNCTIONS-------------------------

	//---------BitMap----------
	Bitmap BitmapLoad(const std::string& path);
	Bitmap BitmapCreate(unsigned short w, unsigned short h);
	Bitmap BitmapCopy(Bitmap bmp);
	Bitmap BitmapClear(Bitmap bmp, Color c);
	void BitmapDestroy(Bitmap bmp);
	Bitmap BitmapGetScreen(void);
	int BitmapGetWidth(Bitmap bmp);
	int BitmapGetHeight(Bitmap bmp);
	void BitmapBlit(Bitmap src, const Rect& from, Bitmap dest, const Point& to);
	template<typename Tfunc>
	void BitmapAccessPixels(Bitmap bmp, const Tfunc& f);

	//---------Color------------
	extern void SetPalette(RGB* palette);
	extern Color Make8(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make16(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make24(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha);
	Color GetPixel32(unsigned char* mem);

	void init(void);

	void SetTile(TileMap* m, Dim col, Dim row, Index index);

	Index GetTile(const TileMap* m, Dim col, Dim row);

	bool ReadTextMap(TileMap* m, string filename);

	void PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile);

	void TileTerrainDisplay(TileMap* map, Bitmap dest, const Rect& viewWin, const Rect& displayArea);
	void TileTerrainDisplay(TileMap* map, Bitmap dest, ViewData& view);

	int GetMapPixelWidth(void);
	int GetMapPixelHeight(void);

	void Scroll(Rect* viewWin, int dx, int dy);

	bool CanScrollHoriz(const Rect& viewWin, int dx);

	bool CanScrollVert(const Rect& viewWin, int dy);

	static void FilterScrollDistance(
		int viewStartCoord,// x  or y
		int viewSize,// w  or h
		int *d,// dx or dy
		int maxMapSize// w or h
	);

	void FilterScroll(const Rect& viewWin, int *dx, int *dy);

	void ScrollWithBoundsCheck(Rect* viewWin, int dx, int dy);

	void setToStartOfMap(Rect* viewWin);

	void SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index);
	
	GridIndex GetGridTile(const GridMap* m, Dim col, Dim row);

	void SetSolidGridTile(GridMap* m, Dim col, Dim row);
	
	void SetEmptyGridTile(GridMap* m, Dim col, Dim row);
	
	void SetGridTileFlags(GridMap* m, Dim col, Dim row, GridIndex flags);
	
	void SetGridTileTopSolidOnly(GridMap* m, Dim col, Dim row);
	
	bool CanPassGridTile(GridMap* m, Dim col, Dim row, GridIndex flags);

	void FilterGridMotion(GridMap* m, const Rect& r, int* dx, int* dy);

	void FilterGridMotionLeft(GridMap* m, const Rect& r, int* dx);

	void FilterGridMotionRight(GridMap* m, const Rect& r, int* dx);

	void FilterGridMotionUp(GridMap* m, const Rect& r, int* dx);

	void FilterGridMotionDown(GridMap* m, const Rect& r, int* dx);

	extern bool IsTileIndexAssumedEmpty(Index index);
	void ComputeTileGridBlocks1(const TileMap* map, GridIndex* grid);

	bool IsTileColorEmpty(Color c);

	void ComputeTileGridBlocks2(const TileMap* map, GridIndex* grid, Bitmap tileSet, Color transColor, unsigned char solidThreshold);

	void ComputeGridBlock(GridIndex*& grid, Index index, Bitmap tileElem, Bitmap gridElem, Bitmap tileSet, Color transColor, unsigned char solidThreshold);
	
	bool ComputeIsGridIndexEmpty(Bitmap gridElement, Color transColor, unsigned char solidThreshold);
}

#endif // !APP_H