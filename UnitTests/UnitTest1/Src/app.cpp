#include "app.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
app::Bitmap dpyBuffer = nullptr;
bool dpyChanged = true;
app::Dim dpyX = 0, dpyY = 0;
app::Bitmap tiles;
int widthInTiles = 0, heightInTiles = 0;
app::Rect viewWin;
app::Rect displayArea;

/*Pre caching*/
unsigned short divIndex[TILE_SET_WIDTH * TILE_SET_HEIGHT];
unsigned short modIndex[TILE_SET_WIDTH * TILE_SET_HEIGHT];

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

void app::App::Initialise(void) {
	viewWin = app::Rect{ 0, 0, VIEW_WIN_X, VIEW_WIN_Y };
	displayArea = app::Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
	if (!al_init()) {
		std::cout << "ERROR: Could not init allegro\n";
		assert(false);
	}
	display = al_create_display(displayArea.w, displayArea.h);
	queue = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_init_image_addon();
	dpyBuffer = app::BitmapCreate(displayArea.w, displayArea.h);

	for (int i = 0; i < TILE_SET_WIDTH * TILE_SET_HEIGHT; ++i) {
		divIndex[i] = (i / TILE_SET_WIDTH) * TILE_HEIGHT;	//y
		modIndex[i] = (i % TILE_SET_WIDTH) * TILE_WIDTH;    //x
	}
}

bool done() {
	//TODO
	return false;
}

void render() {
	app::TileTerrainDisplay(&map, al_get_backbuffer(display), viewWin, displayArea);

	al_flip_display();
}

void input() {
	if (!al_is_event_queue_empty(queue)) {
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			al_get_mouse_cursor_position(&mouse_x, &mouse_y);
			al_get_mouse_state(&mouse_state);
			if (mouse_state.buttons & 1) {
				app::ScrollWithBoundsCheck(&viewWin, prev_mouse_x - mouse_x, prev_mouse_y - mouse_y);
			}
			prev_mouse_x = mouse_x;
			prev_mouse_y = mouse_y;
		}
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_HOME) {
			app::setToStartOfMap(&viewWin);
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_END) {
			app::ScrollWithBoundsCheck(&viewWin, app::GetMapPixelWidth(), app::GetMapPixelHeight());
		}
	}
}

void app::App::Load(void) {
	tiles = app::BitmapLoad(".\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\UnitTests\\UnitTest1\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");

	game.SetDone(done);
	game.SetRender(render);
	game.SetInput(input);
}
void app::App::Clear(void) {
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(dpyBuffer);
}

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
	return (index * TILE_WIDTH) % app::BitmapGetWidth(tiles);
}

app::Dim TileY3(app::Index index) {
	//return MUL_TILE_HEIGHT(GetRow(index));
	//return GetRow(index) * TILE_HEIGHT;
	//return index & TILEY_MASK;
	return ((index * TILE_HEIGHT) / app::BitmapGetWidth(tiles)) * TILE_HEIGHT;
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
				PutTile(dpyBuffer, MUL_TILE_WIDTH(col - startCol), MUL_TILE_HEIGHT(row - startRow), tiles, GetTile(map, row, col));
			}
		}
	}

	BitmapBlit(dpyBuffer, { dpyX, dpyY, viewWin.w, viewWin.h }, dest, { displayArea.x, displayArea.y }); /*THIS IS WRONG, dpyBuffer == dest*/
}


void app::Scroll(Rect* viewWin, int dx, int dy) {
	viewWin->x += dx;
	viewWin->y += dy;
	dpyChanged = true;
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
	return widthInTiles * TILE_WIDTH > displayArea.w ? widthInTiles * TILE_WIDTH : displayArea.w;
}

int app::GetMapPixelHeight(void) {
	return heightInTiles * TILE_HEIGHT > displayArea.h ? heightInTiles * TILE_HEIGHT : displayArea.h;
}

void app::setToStartOfMap(Rect* viewWin) {
	viewWin->x = 0;
	viewWin->y = 0;
	dpyChanged = true;
}