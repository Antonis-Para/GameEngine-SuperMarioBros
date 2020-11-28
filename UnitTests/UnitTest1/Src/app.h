#include <functional>
#include <fstream>
#include <string>
#include <iostream>

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
	typedef void* Bitmap;

	//--------------------STRUCTS---------------------------
	struct Rect{ 
		int x, y, w, h; 
	};
	struct Point{ 
		int x, y; 
	};

	//--------------------GLOBAL VARS-----------------------
	static TileMap map; 
	Bitmap dpyBuffer = nullptr;
	bool dpyChanged = true;
	Dim dpyX = 0, dpyY= 0;

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
		template <typename Tfunc>
		void SetRender(const Tfunc& f) { render = f; }
		// rest of setters are similary defined
		void Render(void) { Invoke(render); }
		void ProgressAnimations(void) { Invoke(anim); }
		void Input(void) { Invoke(input); }
		void AI(void) { Invoke(ai); }
		void Physics(void) { Invoke(physics); }
		void CollisionChecking(void) { Invoke(collisions); }
		void CommitDestructions(void) { Invoke(destruct); }
		void UserCode(void) { Invoke(user); }
		bool IsFinished(void) const { return !done(); }
		void MainLoop(void);
		void MainLoopIteration(void);
	};


	class App {
	protected:
		Game game;

	public:
		virtual void	Initialise (void) = 0;
		virtual void	Load (void) = 0;
		virtual void	Run (void) { game.MainLoop(); }
		virtual void	RunIteration (void)
 						{ game.MainLoopIteration(); }
		Game&			GetGame (void) { return game; }
		const Game&		GetGame (void) const { return game; }
		virtual void	Clear (void) = 0;
		void Main (void) {
			Initialise();
			Load();
			Run();
			Clear();
		}
	};

	//--------------------FUNCTIONS-------------------------
	void SetTile(TileMap* m, Dim col, Dim row, Index index) {
		(*m)[row][col] = index;
	}

	Index GetTile(const TileMap* m, Dim col, Dim row) {
		return (*m)[row][col];
	}

	bool ReadTextMap(TileMap* m, string filename) {
		string line, token, delimiter = ",";
		size_t pos = 0;
		ifstream csvFile(filename);
		int x = 0, y = 0;

		if (csvFile.is_open()) {
			while (getline(csvFile, line)) {
				while ((pos = line.find(delimiter)) != string::npos) {
					token = line.substr(0, pos);
					SetTile(m, x, y, stoi(token));
					x++;
					line.erase(0, pos + delimiter.length());
				}
				y++;
			}
			csvFile.close();
			return true;
		}
		else {
			cout << "Unable to open file " << filename << endl;
		}

		return false;
	}

	Dim TileX3(Index index) {
		return index >> TILEX_SHIFT;
	}

	Dim TileY3(Index index) {
		return index & TILEY_MASK;
	}

	void BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to);

	void PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile) {
		BitmapBlit(tiles, Rect{ TileX3(tile), TileY3(tile), TILE_WIDTH, TILE_HEIGHT}, dest, Point{x, y});
	}

	void TileTerrainDisplay(TileMap* map, Bitmap dest, const Rect& viewWin, const Rect& displayArea) {
		if(dpyChanged) {
			auto startCol = DIV_TILE_WIDTH(viewWin.x);
			auto startRow = DIV_TILE_HEIGHT(viewWin.y);
			auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
			auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.y - 1);
			dpyX = MOD_TILE_WIDTH(viewWin.x);
			dpyY = MOD_TILE_WIDTH(viewWin.y);
			dpyChanged = false;
			for(Dim row = startRow; row <= endRow; ++row)
				for(Dim col = startCol; col <= endCol; ++col)
					PutTile(dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(map, row, col));
		}
		
		BitmapBlit(dpyBuffer, {dpyX, dpyY, viewWin.w, viewWin.h}, dest, {displayArea.x, displayArea.y});
	}

	int GetMapPixelWidth(void);
	int GetMapPixelHeight(void);

	void Scroll(Rect* viewWin, int dx, int dy) {
		viewWin->x += dx;
		viewWin->y += dy;
	}

	bool CanScrollHoriz(const Rect& viewWin, int dx) {
		return viewWin.x >= -dx && (viewWin.x + viewWin.w + dx) <= GetMapPixelWidth();
	}

	bool CanScrollVert(const Rect& viewWin, int dy) {
		return viewWin.y >= -dy && (viewWin.y + viewWin.h + dy) <= GetMapPixelHeight();
	}

	static void FilterScrollDistance(
		int viewStartCoord,// x  or y
		int viewSize,// w  or h
		int *d,// dx or dy
		int maxMapSize// w or h
		) {
			auto val= *d + viewStartCoord;
			if(val< 0)
				*d = viewStartCoord;
			else if((val+ viewSize) >= maxMapSize)
				*d = maxMapSize - (viewStartCoord + viewSize);
	}

	void FilterScroll(const Rect& viewWin, int *dx, int *dy) {
		FilterScrollDistance(viewWin.x, viewWin.w, dx,  GetMapPixelWidth());
		FilterScrollDistance(viewWin.y, viewWin.h, dy,  GetMapPixelHeight());
	}

	void ScrollWithBoundsCheck(Rect* viewWin, int dx, int dy) {
		FilterScroll(*viewWin, &dx, &dy);
		Scroll(viewWin, dx, dy);
	}
}