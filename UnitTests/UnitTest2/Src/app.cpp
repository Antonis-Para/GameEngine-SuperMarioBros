#include "app.h"
#include "Grid.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
app::ViewData view;
app::Bitmap tiles;
int widthInTiles = 0, heightInTiles = 0;
bool closeWindowClicked = false;
app::Character character1;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
bool gridOn = true;

/*Pre caching*/
app::Index* divIndex;
app::Index* modIndex;

app::TileMap map;
ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;

extern GridMap grid;

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
	al_init_primitives_addon();
	view.dpyBuffer = app::BitmapCreate(view.displayArea.w + TILE_WIDTH, view.displayArea.h + TILE_HEIGHT);//it may start on half of the first tile (row) and end on half of the last tile row
	
	timer = al_create_timer(1.0 / 60);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);

	character1.potition = {120, 150, 16, 16};
}


bool app::TileColorsHolder::In(Color c) const {
	return true;
}

bool done() {
	return !closeWindowClicked;
}

void render() {
	app::TileTerrainDisplay(&map, al_get_backbuffer(display), view.viewWin, view.displayArea);

	al_draw_rectangle(character1.potition.x, character1.potition.y, character1.potition.x + character1.potition.w, character1.potition.y + character1.potition.h, {10, 10, 10, 10}, 2.0f);

	al_flip_display();
}

void input() {
	if (!al_is_event_queue_empty(queue)) {
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
			al_get_mouse_cursor_position(&mouse_x, &mouse_y);
			al_get_mouse_state(&mouse_state);
			if (mouse_state.buttons & 1) {
				int move_x = prev_mouse_x - mouse_x;
				int move_y = prev_mouse_y - mouse_y;
				if (app::characterStaysInFrame(&character1, &move_x, &move_y)) {
					app::ScrollWithBoundsCheck(&view.viewWin, &move_x, &move_y);
					app::moveCharacter(&character1, -move_x, -move_y);
				}
			}
			prev_mouse_x = mouse_x;
			prev_mouse_y = mouse_y;
		}
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_TILDE) {
			gridOn = !gridOn;
		}
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			keys[event.keyboard.keycode] = true;
		}
		else if (event.type == ALLEGRO_EVENT_KEY_UP) {
			keys[event.keyboard.keycode] = false;
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			closeWindowClicked = true;
		}
		if (event.type == ALLEGRO_EVENT_TIMER) {
			if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP]) {
				if (character1.potition.y > 0) {
					character1.potition.y += view.viewWin.y;
					character1.potition.x += view.viewWin.x;
					app::moveCharacterWithFilter(&character1, 0, -CHARACTER_MOVE_SPEED);
					character1.potition.x -= view.viewWin.x;
					character1.potition.y -= view.viewWin.y;
				}
			}
			if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN]) {
				if (character1.potition.y + character1.potition.h < view.viewWin.h) {
					character1.potition.y += view.viewWin.y;
					character1.potition.x += view.viewWin.x;
					app::moveCharacterWithFilter(&character1, 0, CHARACTER_MOVE_SPEED);
					character1.potition.x -= view.viewWin.x;
					character1.potition.y -= view.viewWin.y;
				}
			}
			if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
				if (character1.potition.x > 0) {
					character1.potition.x += view.viewWin.x;
					character1.potition.y += view.viewWin.y;
					app::moveCharacterWithFilter(&character1, -CHARACTER_MOVE_SPEED, 0);
					character1.potition.y -= view.viewWin.y;
					character1.potition.x -= view.viewWin.x;
				}
			}
			if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
				if (character1.potition.x + character1.potition.w < view.viewWin.w) {
					character1.potition.x += view.viewWin.x;
					character1.potition.y += view.viewWin.y;
					app::moveCharacterWithFilter(&character1, CHARACTER_MOVE_SPEED, 0);
					character1.potition.y -= view.viewWin.y;
					character1.potition.x -= view.viewWin.x;
				}
			}
			if (keys[ALLEGRO_KEY_HOME]) {
				app::setToStartOfMap(&view.viewWin);
			}
			if (keys[ALLEGRO_KEY_END]) {
				app::ScrollWithBoundsCheck(&view.viewWin, app::GetMapPixelWidth(), app::GetMapPixelHeight());
			}
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
	tiles = app::BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");
}

void loadMap2() {
	tiles = app::BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\MagicLand\\magiclanddizzy_tiles.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\MagicLand\\MagicLand.csv");
}

void loadMap3() {
	tiles = app::BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Outside\\buch-outdoor.png");
	assert(tiles != NULL);

	app::ReadTextMap(&map, ".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Outside\\orthogonal-outside_Ground.csv");
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

	ComputeTileGridBlocks1(&map, &grid[0][0]);

	/*for (auto row = 0; row < 480 * 4; ++row) {
		for (auto col = 0; col < 640 * 4; ++col) {
			cout << (int)grid[row][col] << " ";
		}
		cout << endl;
	}*/
}
void app::MainApp::Clear(void) {
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(view.dpyBuffer);
	exit(0);
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
	return MUL_TILE_WIDTH(index) % app::BitmapGetWidth(tiles);
}

app::Dim TileY3(app::Index index) {
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

void app::ScrollWithBoundsCheck(Rect* viewWin, int *dx, int *dy) {
	FilterScroll(*viewWin, dx, dy);
	Scroll(viewWin, *dx, *dy);
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

void app::moveCharacter(Character *character, int dx, int dy) {
	character->potition.x += dx;
	character->potition.y += dy;
}

void app::moveCharacterWithFilter(Character* character, int dx, int dy) {
	if(gridOn)
		FilterGridMotion(&grid, character->potition, &dx, &dy);
	character->potition.x += dx;
	character->potition.y += dy;
}

bool app::characterStaysInFrame(Character *character, int *dx, int *dy) {
	return !(character->potition.x - *dx < 0
			|| character->potition.x + character->potition.w - *dx > view.displayArea.w
			|| character->potition.y - *dy < 0
			|| character->potition.y + character->potition.h - *dy > view.displayArea.h);
}