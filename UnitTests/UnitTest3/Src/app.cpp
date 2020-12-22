#include "app.h"
#include "GridLayer.h"
#include <iostream>
#include <fstream>
#include "Bitmap.h"

using namespace std;

class TileLayer tilelayer;
class GridLayer gridlayer;

//--------------------GLOBAL VARS-----------------------
//app::ViewData view;
Rect displayArea = Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
int widthInTiles = 0, heightInTiles = 0;
bool closeWindowClicked = false;
app::Character character1;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
bool gridOn = true;
Bitmap tiles;

//TileMap map;
ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;

//extern GridMap grid;

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
	display = al_create_display(displayArea.w, displayArea.h);
	queue = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_init_image_addon();
	al_init_primitives_addon();
	
	timer = al_create_timer(1.0 / 60);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);

	character1.potition = {120, 150, 16, 16};
}

/*
bool app::TileColorsHolder::In(Color c) const {
	return colors.find(c) != colors.end();
}*/

/*
void app::TileColorsHolder::Insert(Bitmap bmp, Index index) {
	if (indices.find(index) == indices.end()) {
		indices.insert(index);
		BitmapAccessPixels(
			bmp,
			[this](PixelMemory mem)
			{ colors.insert(GetPixel32(mem)); }
		);
	}
}*/

bool done() {
	return !closeWindowClicked;
}

void render() {
	tilelayer.TileTerrainDisplay(al_get_backbuffer(display), displayArea, tiles);

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
					tilelayer.ScrollWithBoundsCheck(&move_x, &move_y);
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
					character1.potition.y += tilelayer.GetViewWindow().y;
					character1.potition.x += tilelayer.GetViewWindow().x;
					app::moveCharacterWithFilter(&character1, 0, -CHARACTER_MOVE_SPEED);
					character1.potition.x -= tilelayer.GetViewWindow().x;
					character1.potition.y -= tilelayer.GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN]) {
				if (character1.potition.y + character1.potition.h < tilelayer.GetViewWindow().h) {
					character1.potition.y += tilelayer.GetViewWindow().y;
					character1.potition.x += tilelayer.GetViewWindow().x;
					app::moveCharacterWithFilter(&character1, 0, CHARACTER_MOVE_SPEED);
					character1.potition.x -= tilelayer.GetViewWindow().x;
					character1.potition.y -= tilelayer.GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
				if (character1.potition.x > 0) {
					character1.potition.y += tilelayer.GetViewWindow().y;
					character1.potition.x += tilelayer.GetViewWindow().x;
					app::moveCharacterWithFilter(&character1, -CHARACTER_MOVE_SPEED, 0);
					character1.potition.x -= tilelayer.GetViewWindow().x;
					character1.potition.y -= tilelayer.GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
				if (character1.potition.x + character1.potition.w < tilelayer.GetViewWindow().w) {
					character1.potition.y += tilelayer.GetViewWindow().y;
					character1.potition.x += tilelayer.GetViewWindow().x;
					app::moveCharacterWithFilter(&character1, CHARACTER_MOVE_SPEED, 0);
					character1.potition.x -= tilelayer.GetViewWindow().x;
					character1.potition.y -= tilelayer.GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_HOME]) {
				tilelayer.SetViewWindow({0, 0, tilelayer.GetViewWindow().w , tilelayer.GetViewWindow().h}); //set to start of map
			}
			if (keys[ALLEGRO_KEY_END]) {
				tilelayer.ScrollWithBoundsCheck(app::GetMapPixelWidth(), app::GetMapPixelHeight());
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
	tiles = BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\overworld_tileset_grass.png");
	assert(tiles != NULL);

	app::ReadTextMap(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\map1_Kachelebene 1.csv");
}

void loadMap2() {
	tiles = BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\MagicLand\\magiclanddizzy_tiles.png");
	assert(tiles != NULL);

	app::ReadTextMap(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\MagicLand\\MagicLand.csv");
}

void loadMap3() {
	tiles = BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Outside\\buch-outdoor.png");
	assert(tiles != NULL);

	app::ReadTextMap(".\\hy-454-super-mario\\UnitTests\\UnitTest2\\Media\\Outside\\orthogonal-outside_Ground.csv");
}

void loadSuperMarioMap() {
	tiles = BitmapLoad(".\\hy-454-super-mario\\UnitTests\\UnitTest3\\UnitTest3Media\\super_mario_tiles.png");
	assert(tiles != NULL);

	app::ReadTextMap(".\\hy-454-super-mario\\UnitTests\\UnitTest3\\UnitTest3Media\\super_mario_world_Background.csv");
}
//--------------------------------------

set <Index> solids;
void loadSolidTiles(string path) {
	int tile;

	std::ifstream infile(path);

	while (infile >> tile) {
		solids.insert(tile);
	}
}

void app::MainApp::Load(void) {
	loadSuperMarioMap();

	int tilesw = DIV_TILE_WIDTH(BitmapGetWidth(tiles));
	int tilesh = DIV_TILE_HEIGHT(BitmapGetHeight(tiles));
	// for map2 -> 24x44 but tilesw = 49 & tilesh = 26 (???????)

	tilelayer.Allocate();
	tilelayer.AllocateCaching(tilesw, tilesh);

	game.SetDone(done);
	game.SetRender(render);
	game.SetInput(input);

	loadSolidTiles(".\\UnitTests\\UnitTest2\\Media\\Overworld_GrassBiome\\solid_tiles.txt");
	gridlayer.ComputeTileGridBlocks1(&tilelayer);

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
	al_destroy_bitmap(tilelayer.GetBitmap());
	//TODO destroy grid, tiles
	exit(0);
}

void app::App::Main(void) {
	Initialise();
	Load();
	Run();
	Clear();
}

/*--------------------FUNCTIONS-------------------------*/

bool app::ReadTextMap(string filename) {
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
				tilelayer.SetTile(x, y, val);
				x++;
				line.erase(0, pos + delimiter.length());
			}
			stringstream ss(line);
			int val;
			ss >> val;
			tilelayer.SetTile(x, y, val);
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

Index GetCol(Index index)
{
	return index >> 4;
}
Index GetRow(Index index)
{
	return index & 0xF0;
}


Dim TileX3(Index index) {
	return MUL_TILE_WIDTH(index) % BitmapGetWidth(tiles);
}

Dim TileY3(Index index) {
	return MUL_TILE_HEIGHT(MUL_TILE_HEIGHT(index) / BitmapGetWidth(tiles));
}

int app::GetMapPixelWidth(void) {
	return widthInTiles * TILE_WIDTH > displayArea.w ? widthInTiles * TILE_WIDTH : displayArea.w;
}

int app::GetMapPixelHeight(void) {
	return heightInTiles * TILE_HEIGHT > displayArea.h ? heightInTiles * TILE_HEIGHT : displayArea.h;
}


void app::moveCharacter(Character *character, int dx, int dy) {
	character->potition.x += dx;
	character->potition.y += dy;
}

void app::moveCharacterWithFilter(Character* character, int dx, int dy) {
	if(gridOn)
		gridlayer.FilterGridMotion(character->potition, &dx, &dy);
	character->potition.x += dx;
	character->potition.y += dy;
}

bool app::characterStaysInFrame(Character *character, int *dx, int *dy) {
	return !(character->potition.x - *dx < 0
			|| character->potition.x + character->potition.w - *dx > displayArea.w
			|| character->potition.y - *dy < 0
			|| character->potition.y + character->potition.h - *dy > displayArea.h);
}