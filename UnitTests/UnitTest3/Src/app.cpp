#include "app.h"
#include "Bitmap.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
class TileLayer* action_layer;
class CircularBackground* circular_background;

Rect displayArea = Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
int widthInTiles = 0, heightInTiles = 0;
unsigned int total_tiles;
bool closeWindowClicked = false;
app::Character player1;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
bool gridOn = true;
Bitmap characters = nullptr;

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;

std::map<string, app::Character> prefix_character;

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

void app::App::RunIteration(void) {
	game.MainLoopIteration();
}

app::Game& app::App::GetGame(void) {
	return game;
}

const app::Game& app::App::GetGame(void) const { return game; }

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
	circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);
	action_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);

	BitmapBlit(player1.stand_right, { 0, 0, player1.potition.w, player1.potition.h }, al_get_backbuffer(display), {player1.potition.x, player1.potition.y});

	al_flip_display();
}

void input() {
	if (!al_is_event_queue_empty(queue)) {
		al_wait_for_event(queue, &event);

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
				if (player1.potition.y > 0) {
					player1.potition.y += action_layer->GetViewWindow().y;
					player1.potition.x += action_layer->GetViewWindow().x;
					app::moveCharacterWithFilter(&player1, 0, -CHARACTER_MOVE_SPEED);
					player1.potition.x -= action_layer->GetViewWindow().x;
					player1.potition.y -= action_layer->GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN]) {
				if (player1.potition.y + player1.potition.h < action_layer->GetViewWindow().h) {
					player1.potition.y += action_layer->GetViewWindow().y;
					player1.potition.x += action_layer->GetViewWindow().x;
					app::moveCharacterWithFilter(&player1, 0, CHARACTER_MOVE_SPEED);
					player1.potition.x -= action_layer->GetViewWindow().x;
					player1.potition.y -= action_layer->GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
				if (player1.potition.x > 0) {
					player1.potition.y += action_layer->GetViewWindow().y;
					player1.potition.x += action_layer->GetViewWindow().x;
					app::moveCharacterWithFilter(&player1, -CHARACTER_MOVE_SPEED, 0);
					player1.potition.x -= action_layer->GetViewWindow().x;
					player1.potition.y -= action_layer->GetViewWindow().y;
				}
			}
			if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
				if (player1.potition.x + player1.potition.w < action_layer->GetViewWindow().w) {
					player1.potition.y += action_layer->GetViewWindow().y;
					player1.potition.x += action_layer->GetViewWindow().x;
					app::moveCharacterWithFilter(&player1, CHARACTER_MOVE_SPEED, 0);
					player1.potition.x -= action_layer->GetViewWindow().x;
					player1.potition.y -= action_layer->GetViewWindow().y;
					int move_x = CHARACTER_MOVE_SPEED;
					int move_y = 0;
					if (app::characterStaysInCenter(&player1, &move_x)) {
						action_layer->ScrollWithBoundsCheck(&move_x, &move_y);
						circular_background->Scroll(move_x);
						app::moveCharacter(&player1, -move_x, -move_y);
					}
				}
			}
		}
	}
	
}

void loadMap(string path) {
	app::ReadTextMap(action_layer, path);
}

//--------------------------------------

void loadSolidTiles(TileLayer *layer) {

	string temp = "", token = "", delimiter = " ";
	size_t pos = 0;


	ALLEGRO_CONFIG* config = al_load_config_file(".\\UnitTests\\UnitTest3\\config.ini");
	assert(config != NULL);

	temp = al_get_config_value(config, "tiles", "solid");
	while ((pos = temp.find(delimiter)) != string::npos) {
		token = temp.substr(0, pos);
		stringstream ss(token);
		int tile;
		ss >> tile;
		layer->insertSolid(tile);
		temp.erase(0, pos + delimiter.length());
	}
	token = temp;
	stringstream ss(token);
	int tile;
	ss >> tile;
	layer->insertSolid(tile);
}

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
}

void app::MainApp::Load(void) {

	ALLEGRO_CONFIG* config = al_load_config_file(".\\UnitTests\\UnitTest3\\config.ini");
	assert(config != NULL);

	Bitmap tiles = BitmapLoad(al_get_config_value(config, "paths", "tiles_path"));
	assert(tiles != NULL);

	characters = BitmapLoad(al_get_config_value(config, "paths", "characters_path"));
	assert(characters != NULL);

	action_layer = new TileLayer(MAX_HEIGHT, MAX_WIDTH, tiles);
	action_layer->Allocate();

	int tilesw = DIV_TILE_WIDTH(BitmapGetWidth(tiles)); //tileset width
	int tilesh = DIV_TILE_HEIGHT(BitmapGetHeight(tiles)); //tileset height
	total_tiles = tilesw * tilesh;

	action_layer->InitCaching(tilesw, tilesh);
	loadMap(al_get_config_value(config, "paths", "action_layer_path"));

	circular_background = new CircularBackground(tiles, al_get_config_value(config, "paths", "circular_backround_path"));

	game.SetDone(done);
	game.SetRender(render);
	game.SetInput(input);

	loadSolidTiles(action_layer);
	action_layer->ComputeTileGridBlocks1();

	initialize_prefix_character(config, "Mario_small");

	player1 = prefix_character["Mario_small"];
}

void app::App::Run(void) {
	game.MainLoop();
}

void app::MainApp::Clear(void) {
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(action_layer->GetBitmap());
	//TODO destroy grid, tiles, background
	delete action_layer;
	exit(0);
}

void app::App::Main(void) {
	Initialise();
	Load();
	Run();
	Clear();
}

/*--------------------FUNCTIONS-------------------------*/

bool app::ReadTextMap(class TileLayer* layer, string filename) {
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
				if (val == -1) {
					layer->SetTile(x, y, total_tiles); //"crete" an extra tile which is going to be identified as the trasnparent tile
				}
				else {
					layer->SetTile(x, y, val);
				}
				x++;
				line.erase(0, pos + delimiter.length());
			}
			stringstream ss(line);
			int val;
			ss >> val;
			layer->SetTile(x, y, val);
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


//Dim TileX3(Index index) {
//	return MUL_TILE_WIDTH(index) % BitmapGetWidth(tiles);
//}
//
//Dim TileY3(Index index) {
//	return MUL_TILE_HEIGHT(MUL_TILE_HEIGHT(index) / BitmapGetWidth(tiles));
//}

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
		action_layer->GetGrid()->FilterGridMotion(character->potition, &dx, &dy);
	character->potition.x += dx;
	character->potition.y += dy;
}

bool app::characterStaysInFrame(Character *character, int *dx, int *dy) {
	return !(character->potition.x - *dx < 0
			|| character->potition.x + character->potition.w - *dx > displayArea.w
			|| character->potition.y - *dy < 0
			|| character->potition.y + character->potition.h - *dy > displayArea.h);
}

bool app::characterStaysInCenter(Character* character, int* dx) {
	return character->potition.x + character->potition.w/2 - *dx > displayArea.w/2;
}

void app::initialize_prefix_character(ALLEGRO_CONFIG* config, string char_name) {
	Character character;
	string temp = "", token = "", token2 = "", delimiter = " ", delimiter2 = ",";
	size_t pos = 0, pos2 = 0;
	int x, y;

	character.potition = { 60, 430, 16, 16 };

	temp = al_get_config_value(config, char_name.c_str(), "stand_right");
	pos = temp.find(delimiter);
	token = temp.substr(0, pos);
	stringstream s1(token);
	s1 >> x;
	temp.erase(0, pos + delimiter.length());
	token = temp;
	stringstream s2(token);
	s2 >> y;
	character.stand_right = SubBitmapCreate(characters, Rect{x, y, 16, 16});

	temp = al_get_config_value(config, char_name.c_str(), "stand_left");
	pos = temp.find(delimiter);
	token = temp.substr(0, pos);
	stringstream s3(token);
	s3 >> x;
	temp.erase(0, pos + delimiter.length());
	token = temp;
	stringstream s4(token);
	s4 >> y;
	character.stand_left = SubBitmapCreate(characters, Rect{ x, y, 16, 16 });

	temp = al_get_config_value(config, char_name.c_str(), "walk_right");
	while ((pos2 = temp.find(delimiter2)) != string::npos) {
		token2 = temp.substr(0, pos2);
		pos = temp.find(delimiter);
		token = temp.substr(0, pos);
		stringstream s5(token);
		s5 >> x;
		temp.erase(0, pos + delimiter.length());
		token = temp;
		stringstream s6(token);
		s6 >> y;
		temp.erase(0, pos2 - pos + delimiter2.length());
		character.walk_right.push_back(SubBitmapCreate(characters, Rect{ x, y, 16, 16 }));
	}

	temp = al_get_config_value(config, char_name.c_str(), "walk_left");
	while ((pos2 = temp.find(delimiter2)) != string::npos) {
		token2 = temp.substr(0, pos2);
		pos = temp.find(delimiter);
		token = temp.substr(0, pos);
		stringstream s7(token);
		s7 >> x;
		temp.erase(0, pos + delimiter.length());
		token = temp;
		stringstream s8(token);
		s8 >> y;
		temp.erase(0, pos2 - pos + delimiter2.length());
		character.walk_left.push_back(SubBitmapCreate(characters, Rect{ x, y, 16, 16 }));
	}

	temp = al_get_config_value(config, char_name.c_str(), "jump_right");
	while ((pos2 = temp.find(delimiter2)) != string::npos) {
		token2 = temp.substr(0, pos2);
		pos = temp.find(delimiter);
		token = temp.substr(0, pos);
		stringstream s9(token);
		s9 >> x;
		temp.erase(0, pos + delimiter.length());
		token = temp;
		stringstream s10(token);
		s10 >> y;
		temp.erase(0, pos2 - pos + delimiter2.length());
		character.jump_right.push_back(SubBitmapCreate(characters, Rect{ x, y, 16, 16 }));
	}

	temp = al_get_config_value(config, char_name.c_str(), "jump_left");
	while ((pos2 = temp.find(delimiter2)) != string::npos) {
		token2 = temp.substr(0, pos2);
		pos = temp.find(delimiter);
		token = temp.substr(0, pos);
		stringstream s11(token);
		s11 >> x;
		temp.erase(0, pos + delimiter.length());
		token = temp;
		stringstream s12(token);
		s12 >> y;
		temp.erase(0, pos2 - pos + delimiter2.length());
		character.jump_left.push_back(SubBitmapCreate(characters, Rect{ x, y, 16, 16 }));
	}

	prefix_character[char_name] = character;
}