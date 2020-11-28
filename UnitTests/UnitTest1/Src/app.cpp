#include "app.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
static app::TileMap map;
app::Bitmap dpyBuffer = nullptr;
bool dpyChanged = true;
app::Dim dpyX = 0, dpyY = 0;
app::Bitmap tiles;


/*--------------------CLASSES---------------------------*/

//-------------Class Game----------------
void app::Game::Invoke(const Action& f) { if (f) f(); }
template <typename Tfunc> void app::Game::SetRender(const Tfunc& f) { render = f; }
void app::Game::ProgressAnimations(void) { Invoke(anim); }
void app::Game::AI(void) { Invoke(ai); }
void app::Game::Physics(void) { Invoke(physics); }
void app::Game::CollisionChecking(void) { Invoke(collisions); }
void app::Game::CommitDestructions(void) { Invoke(destruct); }
void app::Game::UserCode(void) { Invoke(user); }
bool app::Game::IsFinished(void) const { return !done(); }


//-------------Class APP----------------

void app::App::Main(void) {
	Initialise();
	Load();
	Run();
	Clear();
}

/*--------------------FUNCTIONS-------------------------*/

void app::SetTile(TileMap* m, Dim col, Dim row, Index index) {
	(*m)[row][col] = index;
}

app::Index app::GetTile(const TileMap* m, Dim col, Dim row) {
	return (*m)[row][col];
}

bool app::ReadTextMap(TileMap* m, string filename) {
	string line, token, delimiter = ",";
	size_t pos = 0;
	ifstream csvFile(filename);
	int x = 0, y = 0;

	if (csvFile.is_open()) {
		while (getline(csvFile, line)) {
			x = 0;
			while ((pos = line.find(delimiter)) != string::npos) {
				token = line.substr(0, pos);
				stringstream ss(token);
				int val;
				ss >> val;
				SetTile(m, x, y, val);
				x++;
				line.erase(0, pos + delimiter.length());
			}
			stringstream ss(line);
			int val;
			ss >> val;
			SetTile(m, x, y, val);
			y++;
		}
		csvFile.close();
		return true;
	}
	else {
		cout << "Unable to open file " << filename << endl;
		exit(EXIT_FAILURE);
	}

	return false;
}

app::Dim app::TileX3(Index index) {
	return index >> TILEX_SHIFT;
}

app::Dim app::TileY3(Index index) {
	return index & TILEY_MASK;
}

void app::PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile) {
	BitmapBlit(tiles, app::Rect{ TileX3(tile), TileY3(tile), TILE_WIDTH, TILE_HEIGHT }, dest, Point{ x, y });
}

void app::TileTerrainDisplay(TileMap* map, Bitmap dest, const Rect& viewWin, const Rect& displayArea) {
	if (dpyChanged) {
		auto startCol = DIV_TILE_WIDTH(viewWin.x);
		auto startRow = DIV_TILE_HEIGHT(viewWin.y);
		auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
		auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.y - 1);
		dpyX = MOD_TILE_WIDTH(viewWin.x);
		dpyY = MOD_TILE_WIDTH(viewWin.y);
		dpyChanged = false;
		for (Dim row = startRow; row <= endRow; ++row)
			for (Dim col = startCol; col <= endCol; ++col)
				PutTile(dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(map, row, col));
	}

	BitmapBlit(dpyBuffer, { dpyX, dpyY, viewWin.w, viewWin.h }, dest, { displayArea.x, displayArea.y });
}


void app::Scroll(Rect* viewWin, int dx, int dy) {
	viewWin->x += dx;
	viewWin->y += dy;
}

bool app::CanScrollHoriz(const Rect& viewWin, int dx) {
	return viewWin.x >= -dx && (viewWin.x + viewWin.w + dx) <= GetMapPixelWidth();
}

bool app::CanScrollVert(const Rect& viewWin, int dy) {
	return viewWin.y >= -dy && (viewWin.y + viewWin.h + dy) <= GetMapPixelHeight();
}

static void app::FilterScrollDistance(
	int viewStartCoord,// x  or y
	int viewSize,// w  or h
	int *d,// dx or dy
	int maxMapSize// w or h
) {
	auto val = *d + viewStartCoord;
	if (val< 0)
		*d = viewStartCoord;
	else if ((val + viewSize) >= maxMapSize)
		*d = maxMapSize - (viewStartCoord + viewSize);
}

void app::FilterScroll(const Rect& viewWin, int *dx, int *dy) {
	FilterScrollDistance(viewWin.x, viewWin.w, dx, GetMapPixelWidth());
	FilterScrollDistance(viewWin.y, viewWin.h, dy, GetMapPixelHeight());
}

void app::ScrollWithBoundsCheck(Rect* viewWin, int dx, int dy) {
	FilterScroll(*viewWin, &dx, &dy);
	Scroll(viewWin, dx, dy);
}

int app::GetMapPixelWidth(void) {
	//TODO
	return 0;
}

int app::GetMapPixelHeight(void) {
	//TODO
	return 0;
}