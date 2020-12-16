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
#include <allegro5/allegro_primitives.h>
#include "Defines.h"

using namespace std;

namespace app {
	//extern Bitmap tiles;



	//--------------------TYPEDEFS--------------------------
	typedef unsigned short Dim;
	typedef unsigned short Index;
	typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
	typedef ALLEGRO_BITMAP* Bitmap;
	typedef ALLEGRO_COLOR Color;
	typedef unsigned char RGBValue;
	typedef unsigned char Alpha;
	typedef unsigned char* PixelMemory;

	using BitmapAccessFunctor = std::function<void(unsigned char**)>;


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

	struct Character {
		Rect potition;
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
	
	//bool ALLEGRO_COLOR::operator<(const Color& c1, const Color& c2) { return true; }

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
	ALLEGRO_LOCKED_REGION* BitmapLock(Bitmap bmp);
	void BitmapUnlock(Bitmap bmp);
	/*PixelMemory BitmapGetMemory(Bitmap bmp);
	int BitmapGetLineOffset(Bitmap bmp);*/
	template<typename Tfunc>
	void BitmapAccessPixels(Bitmap bmp, const Tfunc& f);
	typedef unsigned char* PixelMemory;

	//---------Color------------
	extern void SetPalette(RGB* palette);
	extern Color Make8(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make16(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make24(RGBValue r, RGBValue g, RGBValue b);
	extern Color Make32(RGBValue r, RGBValue g, RGBValue b, Alpha alpha);
	void ReadPixelColor32(PixelMemory mem, RGBA *c, Alpha *a);
	Color GetPixel32(PixelMemory mem);

	//---------App----------------
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
	void ScrollWithBoundsCheck(Rect* viewWin, int *dx, int *dy);

	void setToStartOfMap(Rect* viewWin);

	void moveCharacter(Character *character, int dx, int dy);
	void moveCharacterWithFilter(Character* character, int dx, int dy);

	bool characterStaysInFrame(Character *character, int *dx, int *dy);
}

//--------------------OVERLOADED OPS--------------------

bool operator<(const app::Color left, const app::Color right);

#endif // !APP_H