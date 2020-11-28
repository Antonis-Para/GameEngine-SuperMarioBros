#include <functional>
#include <fstream>
#include <iostream>
#include "Bitmap.h"

using namespace std;

namespace app {

	//--------------------DEFINES---------------------------
	#define TILE_WIDTH 16
	#define TILE_HEIGHT 16
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

	//--------------------TYPEDEFS--------------------------
	typedef unsigned short Dim;
	typedef unsigned short Index;
	typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];

	//--------------------GLOBAL VARS-----------------------
	static TileMap map; 
	Bitmap dpyBuffer = nullptr;
	bool dpyChanged = true;
	Dim dpyX = 0, dpyY= 0;
	Bitmap tiles;

	//--------------------CLASSES---------------------------
	class Game { 
	public:
		using Action = std::function<void(void)>;
		using Pred = std::function<bool(void)>;
	private:
		Action render, anim, input, ai, physics, destruct, collisions, user;
		Pred done;

		void Invoke(const Action& f) { if (f) f(); }
	public:
		template <typename Tfunc> void SetRender(const Tfunc& f);
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
			virtual void	Initialise (void) = 0;
			virtual void	Load (void) = 0;
			virtual void	Run(void);
			virtual void	RunIteration(void);
			Game&			GetGame(void);
			const Game&		GetGame(void) const;
			virtual void	Clear (void) = 0;
			void Main(void);
	};

	//--------------------FUNCTIONS-------------------------
	void init(void);

	void SetTile(TileMap* m, Dim col, Dim row, Index index);

	Index GetTile(const TileMap* m, Dim col, Dim row);

	bool ReadTextMap(TileMap* m, string filename);

	Dim TileX3(Index index);

	Dim TileY3(Index index);

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