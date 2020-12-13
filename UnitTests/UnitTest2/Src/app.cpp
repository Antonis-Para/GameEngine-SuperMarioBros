#include "app.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
/*app::ViewData views[MAX_VIEWS];
app::Bitmap dpyBuffer = nullptr;
bool dpyChanged = true;
app::Dim dpyX = 0, dpyY = 0;
app::Rect viewWin;
app::Rect displayArea;*/
app::ViewData view;
app::Bitmap tiles;
int widthInTiles = 0, heightInTiles = 0;
bool closeWindowClicked = false;

/*Pre caching*/
app::Index* divIndex;
app::Index* modIndex;

static app::GridMap grid;

static app::TileColorsHolder emptyTileColors;

app::TileMap map;
ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;
/*--------------------CLASSES---------------------------*/

//-------------Class Game----------------
void app::Game::Invoke(const Action& f) { if (f) f(); }
template <typename Tfunc> void app::Game::SetDone(const Tfunc& f) { done = f; }
template <typename Tfunc> void app::Game::SetRender(const Tfunc& f) { render = f; }
template <typename Tfunc> void app::Game::SetInput(const Tfunc& f) { input = f; }
void app::Game::Render(void) { Invoke(render); }
void app::Game::ProgressAnimations(void) { Invoke(anim); }
void app::Game::Input(void) { Invoke(input); }
void app::Game::AI(void) { Invoke(ai); }
void app::Game::Physics(void) { Invoke(physics); }
void app::Game::CollisionChecking(void) { Invoke(collisions); }
void app::Game::CommitDestructions(void) { Invoke(destruct); }
void app::Game::UserCode(void) { Invoke(user); }
bool app::Game::IsFinished(void) const { return !done(); }
void app::Game::MainLoop(void) {
	while (!IsFinished())
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

//-------------Class APP----------------

void app::App::Run(void) {
	game.MainLoop();
}

void app::App::RunIteration(void) {
	game.MainLoopIteration();
}

app::Game& app::App::GetGame(void) {
	return game;
}

const app::Game& app::App::GetGame(void) const { return game; }

void app::MainApp::Initialise(void) {

	if (!al_init()) {
		std::cout << "ERROR: Could not init allegro\n";
		assert(false);
	}
	al_set_new_display_flags(ALLEGRO_WINDOWED);
	display = al_create_display(view.displayArea.w, view.displayArea.h);
	queue = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_init_image_addon();
	view.dpyBuffer = app::BitmapCreate(view.displayArea.w, view.displayArea.h);
	//view.viewWin = app::Rect{ 0, 0, VIEW_WIN_X, VIEW_WIN_Y };
	//view.displayArea = app::Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
}

bool done() {
	return !closeWindowClicked;
}

void render() {
	app::TileTerrainDisplay(&map, al_get_backbuffer(display), view.viewWin, view.displayArea);

	al_flip_display();
}

void input() {
	if (!al_is_event_queue_empty(queue)) {
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			al_get_mouse_cursor_position(&mouse_x, &mouse_y);
			al_get_mouse_state(&mouse_state);
			if (mouse_state.buttons & 1) {
				app::ScrollWithBoundsCheck(&view.viewWin, prev_mouse_x - mouse_x, prev_mouse_y - mouse_y);
			}
			prev_mouse_x = mouse_x;
			prev_mouse_y = mouse_y;
		}
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_HOME) {
			app::setToStartOfMap(&view.viewWin);
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_END) {
			app::ScrollWithBoundsCheck(&view.viewWin, app::GetMapPixelWidth(), app::GetMapPixelHeight());
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			closeWindowClicked = true;
		}
	}
}

//demi-functions for maps
//--------------------------------------

/*			MAP1:
* tile size: 16x16 pixels
* 
* tileset: 12x21 tiles
* tileset: 192x336 pixels
* 
* map: 21x42 tiles
* map: 336x672 pixels
*/
void loadMap1() {
	tiles = app::BitmapLoad(".\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");
}

void loadMap2() {
	tiles = app::BitmapLoad(".\\UnitTests\\UnitTest2\\Media\\MagicLand\\magiclanddizzy_tiles.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\UnitTests\\UnitTest2\\Media\\MagicLand\\MagicLand.csv");
}

void loadMap3() {
	tiles = app::BitmapLoad(".\\UnitTests\\UnitTest2\\Media\\Outside\\buch-outdoor.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\UnitTests\\UnitTest2\\Media\\Outside\\orthogonal-outside_Ground.csv");
}
//--------------------------------------

void app::MainApp::Load(void) {
	loadMap1();

	int tilesw = DIV_TILE_WIDTH(BitmapGetWidth(tiles));
	int tilesh = DIV_TILE_HEIGHT(BitmapGetHeight(tiles));
	// for map2 -> 24x44 but tilesw = 49 & tilesh = 26 (???????)

	divIndex = new Index[tilesw * tilesh];
	modIndex = new Index[tilesw * tilesh];

	for (int i = 0; i < tilesw * tilesh; ++i) {
		divIndex[i] = MUL_TILE_HEIGHT(i / tilesw); //y
		modIndex[i] = MUL_TILE_WIDTH(i % tilesw); //x
	}

	game.SetDone(done);
	game.SetRender(render);
	game.SetInput(input);
}
void app::MainApp::Clear(void) {
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(view.dpyBuffer);
}

void app::App::Main(void) {
	Initialise();
	Load();
	Run();
	Clear();
}

void app::TileColorsHolder::Insert(Bitmap bmp, Index index) {
	if (indices.find(index) == indices.end()) {
		indices.insert(index);
		BitmapAccessPixels(bmp, [this](unsigned char* mem) {
			colors.insert(GetPixel32(mem));
		});
	}
}

bool app::TileColorsHolder::In(Color c) const {
	return colors.find(c) != colors.end();
}

/*--------------------FUNCTIONS-------------------------*/

void app::SetTile(TileMap* m, Dim col, Dim row, Index index) {
	(*m)[row][col] = index;
}

app::Index app::GetTile(const TileMap* m, Dim row, Dim col) {
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
			x++;
			y++;
		}
		widthInTiles = x;
		heightInTiles = y;
		csvFile.close();
		return true;
	}
	else {
		cout << "Unable to open file " << filename << endl;
		exit(EXIT_FAILURE);
	}

	return false;
}

app::Index GetCol(app::Index index)
{
	return index >> 4;
}
app::Index GetRow(app::Index index)
{
	return index & 0xF0;
}


app::Dim TileX3(app::Index index) {
	//return MUL_TILE_WIDTH(GetCol(index));
	//return GetCol(index) * TILE_WIDTH;
	//return index >> TILEX_SHIFT;
	return MUL_TILE_WIDTH(index) % app::BitmapGetWidth(tiles);
}

app::Dim TileY3(app::Index index) {
	//return MUL_TILE_HEIGHT(GetRow(index));
	//return GetRow(index) * TILE_HEIGHT;
	//return index & TILEY_MASK;
	return MUL_TILE_HEIGHT(MUL_TILE_HEIGHT(index) / app::BitmapGetWidth(tiles));
}

app::Dim TileXc(app::Index index) {
	return modIndex[index];
}

app::Dim TileYc(app::Index index) {
	return divIndex[index];
}

void app::PutTile(Bitmap dest, Dim x, Dim y, Bitmap tiles, Index tile) {
	BitmapBlit(tiles, app::Rect{ TileXc(tile), TileYc(tile), TILE_WIDTH, TILE_HEIGHT }, dest, Point{ x, y });
}

void app::TileTerrainDisplay(TileMap* map, Bitmap dest, const Rect& viewWin, const Rect& displayArea) {
	if (view.dpyChanged) {
		auto startCol = DIV_TILE_WIDTH(viewWin.x);
		auto startRow = DIV_TILE_HEIGHT(viewWin.y);
		auto endCol = DIV_TILE_WIDTH(viewWin.x + viewWin.w - 1);
		auto endRow = DIV_TILE_HEIGHT(viewWin.y + viewWin.h - 1);
		view.dpyX = MOD_TILE_WIDTH(viewWin.x);
		view.dpyY = MOD_TILE_HEIGHT(viewWin.y);
		view.dpyChanged = false;
		for (Dim row = startRow; row <= endRow; ++row) {
			for (Dim col = startCol; col <= endCol; ++col) {
				PutTile(view.dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(map, row, col));
			}
		}
	}

	BitmapBlit(view.dpyBuffer, { view.dpyX, view.dpyY, viewWin.w, viewWin.h }, dest, { displayArea.x, displayArea.y });
}

void app::TileTerrainDisplay(TileMap* map, Bitmap dest, ViewData& view) {
	if (view.dpyChanged) {
		auto startCol = DIV_TILE_WIDTH(view.viewWin.x);
		auto startRow = DIV_TILE_HEIGHT(view.viewWin.y);
		auto endCol = DIV_TILE_WIDTH(view.viewWin.x + view.viewWin.w - 1);
		auto endRow = DIV_TILE_HEIGHT(view.viewWin.y + view.viewWin.h - 1);
		view.dpyX = MOD_TILE_WIDTH(view.viewWin.x);
		view.dpyY = MOD_TILE_HEIGHT(view.viewWin.y);
		view.dpyChanged = false;
		for (Dim row = startRow; row <= endRow; ++row) {
			for (Dim col = startCol; col <= endCol; ++col) {
				PutTile(view.dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(map, row, col));
			}
		}
	}

	BitmapBlit(view.dpyBuffer, { view.dpyX, view.dpyY, view.viewWin.w, view.viewWin.h }, dest, { view.displayArea.x, view.displayArea.y });
}

void app::Scroll(Rect* viewWin, int dx, int dy) {
	viewWin->x += dx;
	viewWin->y += dy;
	view.dpyChanged = true;
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
	if (val < 0)
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
	return widthInTiles * TILE_WIDTH > view.displayArea.w ? widthInTiles * TILE_WIDTH : view.displayArea.w;
}

int app::GetMapPixelHeight(void) {
	return heightInTiles * TILE_HEIGHT > view.displayArea.h ? heightInTiles * TILE_HEIGHT : view.displayArea.h;
}

void app::setToStartOfMap(Rect* viewWin) {
	viewWin->x = 0;
	viewWin->y = 0;
	view.dpyChanged = true;
}

void app::SetGridTile(GridMap* m, Dim col, Dim row, GridIndex index) {
	(*m)[row][col] = index;
}

app::GridIndex app::GetGridTile(const GridMap* m, Dim col, Dim row) {
	return(*m)[row][col];
}

void app::SetSolidGridTile(GridMap* m, Dim col, Dim row) {
	SetGridTile(m, col, row, GRID_SOLID_TILE);
}

void app::SetEmptyGridTile(GridMap* m, Dim col, Dim row) {
	SetGridTile(m, col, row, GRID_EMPTY_TILE);
}

void app::SetGridTileFlags(GridMap* m, Dim col, Dim row, GridIndex flags) {
	SetGridTile(m, col, row, flags);
}

void app::SetGridTileTopSolidOnly(GridMap* m, Dim col, Dim row) {
	SetGridTileFlags(m, row, col, GRID_TOP_SOLID_MASK);
}

bool app::CanPassGridTile(GridMap* m, Dim col, Dim row, GridIndex flags) {
	return (GetGridTile(m, row, col) & flags) != 0;
}

void app::FilterGridMotion(GridMap* m, const Rect& r, int* dx, int* dy) {
	assert(abs(*dx) <= GRID_ELEMENT_WIDTH && abs(*dy) <= GRID_ELEMENT_HEIGHT);

	// try horizontal move
	if (*dx < 0)
		FilterGridMotionLeft(m, r, dx);
	else if (*dx > 0)
		FilterGridMotionRight(m, r, dx);

	// try vertical move
	if (*dy < 0)
		FilterGridMotionUp(m, r, dy);
	else if (*dy > 0)
		FilterGridMotionDown(m, r, dy);
}

void app::FilterGridMotionLeft(GridMap* m, const Rect& r, int* dx) {
	auto x1_next = r.x + *dx;
	if (x1_next < 0)
		*dx = -r.x;
	else {
		auto newCol = DIV_GRID_ELEMENT_WIDTH(x1_next);
		auto currCol = DIV_GRID_ELEMENT_WIDTH(r.x);
		if (newCol != currCol) {
			assert(newCol + 1 == currCol); // we really move left
			auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
			auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y + r.h - 1);
			for (auto row = startRow; row <= endRow; ++row)
				if (!CanPassGridTile(m, newCol, row, GRID_RIGHT_SOLID_MASK)) {
					*dx = MUL_GRID_ELEMENT_WIDTH(currCol) - r.x;
					break;
				}
		}
	}
}

void app::FilterGridMotionRight(GridMap* m, const Rect& r, int* dx) {
	auto x2 = r.x + r.w - 1;
	auto x2_next = x2 + *dx;
	if (x2_next >= MAX_PIXEL_WIDTH)
		*dx = MAX_PIXEL_WIDTH - x2;
	else {
		auto newCol = DIV_GRID_ELEMENT_WIDTH(x2_next);
		auto currCol = DIV_GRID_ELEMENT_WIDTH(x2);
		if (newCol != currCol) {
			assert(newCol - 1 == currCol); // we really move right
			auto startRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
			auto endRow = DIV_GRID_ELEMENT_HEIGHT(r.y+ r.h-1);
			for(auto row = startRow; row <= endRow; ++row)
				if(!CanPassGridTile(m, newCol, row, GRID_LEFT_SOLID_MASK)) {
					*dx = MUL_GRID_ELEMENT_WIDTH(newCol) -(x2 + 1);
					break;
				}
		}
	}
}

void app::FilterGridMotionUp(GridMap* m, const Rect& r, int* dy) {
	auto y2 = r.y + r.h - 1;
	auto y2_next = y2 + *dy;
	if (y2_next >= MAX_PIXEL_HEIGHT)
		*dy = MAX_PIXEL_HEIGHT - y2;
	else {
		auto newRow = DIV_GRID_ELEMENT_HEIGHT(y2_next);
		auto currRow = DIV_GRID_ELEMENT_HEIGHT(y2);
		if (newRow != currRow) {
			assert(newRow - 1 == currRow); // we really move down
			auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
			for (auto col = startCol; col <= endCol; ++col)
				if (!CanPassGridTile(m, newRow, col, GRID_BOTTOM_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_HEIGHT(newRow) - (y2 + 1);
					break;
				}
		}
	}
}

void app::FilterGridMotionDown(GridMap* m, const Rect& r, int* dy) {
	auto y1_next = r.y + *dy;
	if (y1_next < 0)
		*dy = -r.y;
	else {
		auto newRow = DIV_GRID_ELEMENT_HEIGHT(y1_next);
		auto currRow = DIV_GRID_ELEMENT_HEIGHT(r.y);
		if (newRow != currRow) {
			assert(newRow + 1 == currRow); // we really move left
			auto startCol = DIV_GRID_ELEMENT_WIDTH(r.x);
			auto endCol = DIV_GRID_ELEMENT_WIDTH(r.x + r.w - 1);
			for (auto col = startCol; col <= endCol; ++col)
				if (!CanPassGridTile(m, newRow, col, GRID_TOP_SOLID_MASK)) {
					*dy = MUL_GRID_ELEMENT_WIDTH(currRow) - r.y;
					break;
				}
		}
	}
}

void app::ComputeTileGridBlocks1(const TileMap* map, GridIndex* grid) {
	for (auto row = 0; row < MAX_HEIGHT; ++row)
		for (auto col = 0; col < MAX_WIDTH; ++col) {
			memset(grid, IsTileIndexAssumedEmpty(GetTile(map, col, row)) ? GRID_EMPTY_TILE : GRID_SOLID_TILE, GRID_ELEMENTS_PER_TILE);
			grid += GRID_ELEMENTS_PER_TILE;
		}
}

bool app::IsTileColorEmpty(Color c) {
	return emptyTileColors.In(c);
}

void app::ComputeTileGridBlocks2(const TileMap* map, GridIndex* grid, Bitmap tileSet, Color transColor, unsigned char solidThreshold) {
	auto tileElem = BitmapCreate(TILE_WIDTH, TILE_HEIGHT);
	auto gridElem = BitmapCreate(GRID_ELEMENT_WIDTH, GRID_ELEMENT_HEIGHT);
	for (auto row = 0; row < MAX_HEIGHT; ++row)
		for (auto col = 0; col < MAX_WIDTH; ++col) {
			auto index = GetTile(map, col, row);
			PutTile(tileElem, 0, 0, tileSet, index);
			if (IsTileIndexAssumedEmpty(index)) {
				emptyTileColors.Insert(tileElem, index);// assume tile colors to be empty
				memset(grid, GRID_EMPTY_TILE, GRID_ELEMENTS_PER_TILE);
				grid += GRID_ELEMENTS_PER_TILE;
			}
			else
				ComputeGridBlock(grid, index, tileElem, gridElem, tileSet, transColor, solidThreshold);
		}
	BitmapDestroy(tileElem);
	BitmapDestroy(gridElem);
}

void app::ComputeGridBlock(GridIndex*& grid, Index index, Bitmap tileElem, Bitmap gridElem, Bitmap tileSet, Color transColor, unsigned char solidThreshold) {
	for (auto i = 0; i < GRID_ELEMENTS_PER_TILE; ++i) {
		auto x = i % GRID_BLOCK_ROWS;
		auto y = i / GRID_BLOCK_ROWS;
		BitmapBlit(tileElem, { x * GRID_ELEMENT_WIDTH, y * GRID_ELEMENT_HEIGHT, GRID_ELEMENT_WIDTH, GRID_ELEMENT_HEIGHT }, gridElem, { 0, 0 });
		auto isEmpty = ComputeIsGridIndexEmpty(tmp2, transColor, solidThreshold);
		*grid++ = isEmpty ? GRID_EMPTY_TILE : GRID_SOLID_TILE;
	}
}

bool app::ComputeIsGridIndexEmpty(Bitmap gridElement, Color transColor, unsigned char solidThreshold) {
	auto n = 0;
	BitmapAccessPixels(gridElement, [transColor, &n](unsigned char* mem) {
		auto c = GetPixel32(mem);
		if (c != transColor && !IsTileColorEmpty(c))
			++n;
		});
	return n <= solidThreshold;
}