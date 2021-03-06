#pragma once

#ifndef APP_H
#define APP_H

#include <functional>
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

	#define MUL_TILE_WIDTH(i) ((i) << 4)
	#define MUL_TILE_HEIGHT(i) ((i) << 4)
	#define DIV_TILE_WIDTH(i) ((i) >> 4)
	#define DIV_TILE_HEIGHT(i) ((i) >> 4)
	#define MOD_TILE_WIDTH(i) ((i) & 15)
	#define MOD_TILE_HEIGHT(i) ((i) & 15)

	#define VIEW_WIN_X 640
	#define VIEW_WIN_Y 480
	#define DISP_AREA_X 640
	#define DISP_AREA_Y 480

	#define MAX_WIDTH 1024
	#define MAX_HEIGHT 256
	#define TILEX_MASK 0xFF00
	#define TILEX_SHIFT 8
	#define TILEY_MASK 0x00FF

	//--------------------TYPEDEFS--------------------------
	typedef unsigned short Dim;
	typedef unsigned short Index;
	typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
	typedef ALLEGRO_BITMAP* Bitmap;
	typedef ALLEGRO_COLOR Color;
	typedef unsigned char RGBValue;
	typedef unsigned char Alpha;

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
			virtual void		Initialise (void);
			virtual void		Load (void);
			virtual void		Run(void);
			virtual void		RunIteration(void);
			Game&				GetGame(void);
			const Game&			GetGame(void) const;
			virtual void		Clear (void);
			void Main(void);
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

	//---------Color------------
	extern void SetPalette(RGB* palette);
	extern Color Make8(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make16(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make24(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha);

	void init(void);

	void SetTile(TileMap* m, Dim col, Dim row, Index index);

	Index GetTile(const TileMap* m, Dim col, Dim row);

	bool ReadTextMap(TileMap* m, string filename);

	void PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile);

	void TileTerrainDisplay(TileMap* map, Bitmap dest, const Rect& viewWin, const Rect& displayArea);

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
}

#endif // !APP_H