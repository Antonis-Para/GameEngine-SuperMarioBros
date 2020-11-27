#include<stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <assert.h>  

using namespace std;

// lecture 5, slide 27
class Game { // app::Game namespace, the mother application
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

// lecture 5, slide 28
void app::Game::MainLoop(void) {
	while(!IsFinished())
		MainLoopIteration();
}

void app::Game::MainLoopIteration(void) {
	Render();
	Input();
	ProgressAnimations();
	AI();
	Physics();
	CollisionChecking();
	CommitDestructions();
	UserCode(); // hook for custom code at end
}

// lecture 6, slide 12
typedef unsigned short Dim;
struct Rect{ int x, y, w, h; };
struct Point{ intx, y; };
enum BitDepth{ bits8= 1, bits16, bits24, bits32};
bool Open(Dimrw, Dimrh, BitDepthdepth);
void Close (void);
Dim GetResWidth(void);
Dim GetResHeight(void);
BitDepth GetDepth(void);

// lecture 6, slide 13
//------------- Map to Allegro -------------
typedef unsigned int Color;
typedef unsigned char RGBValue;
typedef unsigned char Alpha;
struct RGB{ RGBValue r, g, b; };
struct RGBA: public RGB{ RGBValue a; };
typedef RGB Palette[256];
void SetPalette(RGB* palette);
Color Make8  (RGBValue r, RGBValue g, RGBValue b);
Color Make16 (RGBValue r, RGBValue g, RGBValue b);
Color Make24 (RGBValue r, RGBValue g, RGBValue b);
Color Make32 (RGBValue r, RGBValue g, RGBValue b, Alpha alpha=0);

// lecture 6, slide 20
//------------- Map to Allegro -------------
typedef void* Bitmap;
Bitmap BitmapLoad(const std::string& path);
Bitmap BitmapCreate(Dim w, Dim h);
Bitmap BitmapCopy(Bitmap bmp);
Bitmap BitmapClear(Bitmap bmp, Color c);
void BitmapDestroy(Bitmap bmp);
Bitmap BitmapGetScreen(void);
Dim BitmapGetWidth(Bitmap bmp);
Dim BitmapGetHeight(Bitmap bmp);
void BitmapBlit(Bitmap src,  const Rect& from, Bitmap dest, const Point& to);

// lecture 6, slide 24
using BitmapAccessFunctor = std::function<void(PixelMemory*)>;
template<typenameTfunc>
void BitmapAccessPixels(Bitmap bmp, const Tfunc& f) {
	auto result = BitmapLock(bmp);
	assert(result);
	auto mem = BitmapGetMemory(bmp);
	auto offset = BitmapGetLineOffset(bmp);
	for(auto y = BitmapGetHeight(bmp); y--;) {
		auto buff = mem;
		for(auto x = BitmapGetWidth(bmp); x--;) {
			f(buff);
			buff += GetDepth();
		}
		mem += offset;
	}
	BitmapUnlock(bmp);
}

// lecure 6, slide 25
//------------- Map to Allegro -------------
typedef unsigned char* PixelMemory;
bool BitmapLock(Bitmap);
void BitmapUnlock(Bitmap);
PixelMemory BitmapGetMemory(Bitmap);
int BitmapGetLineOffset(Bitmap);
void WritePixelColor8(PixelMemory, RGBValue);
void WritePixelColor16(PixelMemory, constRGB&);
void WritePixelColor24(PixelMemory, constRGB&);
void WritePixelColor32(PixelMemory, constRGB&, Alpha a);
void ReadPixelColor8(PixelMemory, RGBValue*);
void ReadPixelColor16(PixelMemory, RGB*);
void ReadPixelColor24(PixelMemory, RGB*);
void ReadPixelColor32(PixelMemory, RGB*, Alpha*);

// lecture 6, slide 26
#define INVERTED_BYTE(b) (255 -(b))
#define TINTED_BYTE(b, f) ((b) * (f))
void InvertPixelColor32(PixelMemory pixel) {
	// assume RGBA order, low to high byte, alpha retained!
	pixel[0] = INVERTED_BYTE(pixel[0]);
	pixel[1] = INVERTED_BYTE(pixel[1]);
	pixel[2] = INVERTED_BYTE(pixel[2]);
}

void TintPixelColor32(PixelMemory pixel, float f) {
	pixel[0] = TINTED_BYTE(pixel[0], f);
	pixel[1] = TINTED_BYTE(pixel[1], f);
	pixel[2] = TINTED_BYTE(pixel[2], f);
}

void BitmapInvertPixels32(Bitmap bmp) {
	BitmapAccessPixels(bmp,  &InvertPixelColor32);
}

void BitmapTintPixels32(Bitmap bmp, float f) {
	BitmapAccessPixels(bmp, [f](PixelMemory pixel) { TintPixelColor32(pixel, f); });
}

// lecture 6, slide 27
unsigned GetRedShiftRGBA(void);
unsigned GetRedBitMaskRGBA(void);
unsigned GetGreenShiftRGBA(void);
unsigned GetGreenBitMaskRGBA(void);
unsigned GetBlueShiftRGBA(void);
unsigned GetBlueBitMaskRGBA(void);
unsigned GetAlphaShiftRGBA(void);
unsigned GetAlphaBitMaskRGBA(void);

//firstly mask to isolate the RGB component, then shift to get value
RGBValue GetRedRGBA(PixelMemory pixel) {
	Color c = *((Color*) pixel);
	return(c & GetRedBitMaskRGBA()) >> GetRedShiftRGBA();
}

// lecture 6, slide 29
void PutPixel8 (Bitmap b, Dim x, Dim y, Color c) {
	if(!BitmapLock(b))
		assert(false);
	WritePixelColor8(BitmapGetMemory(b) + y * (BitmapGetWidth(b) + BitmapGetLineOffset(b)) + x, (RGBValue) c);
	BitmapUnlock(b);
}

// lecture 6, slide 30
// function table approach
static void PutPixel16(Bitmap b, Dim x, Dim y, Color c);
static void PutPixel24(Bitmap b, Dim x, Dim y, Color c);
static void PutPixel32(Bitmap b, Dim x, Dim y, Color c);
typedef void(*PutPixelFunc)(Bitmap b, Dim x, Dim y, Color c);
static PutPixelFunc putPixelFuncs[] = {PutPixel8, PutPixel16, PutPixel24, PutPixel32};
static PutPixelFunc currPutPixel;

extern void PutPixel(Bitmap b, Dim x, Dim y, Color c) {
	(*currPutPixel)(b, x, y, c);
}

extern void InstallPutPixel(void) {// upon initialisation
	currPutPixel = putPixelFuncs[GetDepth()];
}

// lecture 7, slide 5
//------------- Map to Allegro -------------
void SetColorKey(Color c);
Color GetColorKey(void);
Dim MaskedBlit(Bitmap src, const Rect& from, Bitmap dest, const Point& to);

//lecture 7, slide 9
void BitmapBlitTinted( // <- check support in your library
	Bitmap src, const Rect& from,
	Bitmap dest, const Point& to,
	Color  modulation);

// emulate transparent blending
void BitmapBlitTransparent(Bitmap src, const Rect& from, Bitmap dest, const Point& to, RGBValue alpha) {
	BitmapBlitTinted(src, from, dest, to, Make32(255, 255, 255, alpha));
}

// lecture 7, slides 21 - 26

// lecture 7, slide 34
extern Bitmap GetBackBuffer(void);
extern Color GetBackgroundColor(void);
extern Rect& GetScreenRect(void);
extern void Render(Bitmap target);// do game rendering to target
extern void Vsync(void);// gfx lib function

void Flush(void) {
	BitmapClear(GetBackBuffer(), GetBackgroundColor());// optional
	Render(GetBackBuffer);
	Vsync();
	BitmapBlit(GetBackBuffer(), GetScreenRect(), BitmapGetScreen(), Point{0,0});
}

// lecture 7, slide 36
unsigned char frequencyTime;// e.g 14 msecs για 75 Hz
unsigned int timeToNextRendering = 0xffffffff;
extern uint64_t CurrTime (void);// timer in msecs

void Flush2(void) {
	if(timeToNextRendering >= CurrTime()) {
		Vsync(); // just done with refresh
		auto t = CurrTime();
		Render(GetBackBuffer);
		t = CurrTime() - t; // time required to render
		timeToNextRendering = (CurrTime() + frequencyTime) - t;
	}
}

//lecture 8, slide 11
#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define ROW_MASK 0x0F
#define COL_MASK 0xF0
#define COL_SHIFT 4
typedef unsigned char byte;

byte MakeIndex(byte row, byte col) {
	return(col<< COL_SHIFT) | row;
}

byte GetCol(byte index) {
	return index >> COL_SHIFT;
}

byte GetRow(byte index) {
	return index & ROW_MASK;
}

Dim TileX(byte index) {
	return GetCol(index) * TILE_WIDTH;
}

Dim TileY(byte index) {
	returnGetRow(index) * TILE_HEIGHT;
}

#define MUL_TILE_WIDTH(i) ((i) << 4)
#define MUL_TILE_HEIGHT(i) ((i) << 4)
#define DIV_TILE_WIDTH(i) ((i) >> 4)
#define DIV_TILE_HEIGHT(i) ((i) >> 4)
#define MOD_TILE_WIDTH(i) ((i) & 15)
#define MOD_TILE_HEIGHT(i) ((i) & 15)

Dim TileX2(byte index) {
	return MUL_TILE_WIDTH(GetCol(index));
}

Dim TileY2(byte index) {
	return MUL_TILE_HEIGHT(GetRow(index));
}

// lecture 8, slide 12
typedef unsigned short Index; // [MSB X][LSB Y]
#define TILEX_MASK 0xFF00
#define TILEX_SHIFT 8
#define TILEY_MASK 0x00FF

Index MakeIndex2(byte row, byte col) {
	return (MUL_TILE_WIDTH(col) << TILEX_SHIFT) | MUL_TILE_HEIGHT(row);
}

Dim TileX3(Index index) {
	return index >> TILEX_SHIFT;
}

Dim TileY3(Index index) {
	return index & TILEY_MASK;
}

void PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile) {
	BitmapBlit(tiles, Rect{ TileX3(tile), TileY3(tile), TILE_WIDTH, TILE_HEIGHT}, dest, Point{x, y});
}

// lecture 8, slide 16
// fixed size, game requirements lock these values
#define MAX_WIDTH 1024
#define MAX_HEIGHT 256
typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
static TileMap map; // example of a global static map

void SetTile(TileMap* m, Dim col, Dim row, Index index) {
	(*m)[row][col] = index;
}

Index GetTile(const TileMap* m, Dim col, Dim row) {
	return (*m)[row][col];
}

void WriteBinMap(const TileMap* m, FILE* fp) {
	fwrite(m, sizeof(TileMap), 1, fp);
} // simplistic...

bool ReadBinMap(TileMap* m, FILE* fp) {
	/* binary formatted read, like descent parsing */
}

void WriteTextMap(const TileMap*, FILE* fp) {
	/* custom write in text format */
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

// lecture 8, slide 24
Bitmap dpyBuffer = nullptr;
bool dpyChanged = true;
Dim dpyX = 0, dpyY= 0;

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

// lecture 8, slide 26
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

// lecture 8, slide 27
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

// lecture 8, slide 29
struct ViewData {
	Bitmap dpyBuffer = nullptr;
	bool dpyChanged = true;
	Dim dpyX = 0, dpyY = 0;
	Rect viewWin;
	Rect displayArea;
};

#define MAX_VIEWS 4
ViewData views[MAX_VIEWS];

// refined to accept everything from 'view' parameter
void TileTerrainDisplay(Bitmap dest, ViewData& view);
void DisplayTerrain(void) {
	for(auto i= 0; i < MAX_VIEWS; ++i)
		TileTerrainDisplay(GetBackBuffer(), views[i]);
}

// the rest is for grid