#include "app.h"
#include "Bitmap.h"
#include "Utilities.h"
#include <string>
#include <vector>
#include "Sprite.h"
#include "Animation.h"
#include "Animator.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
class TileLayer* action_layer;
class CircularBackground* circular_background;

Rect displayArea = Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
int widthInTiles = 0, heightInTiles = 0;
unsigned int total_tiles;
bool closeWindowClicked = false;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
ALLEGRO_TIMER* fallingTimer;
bool gridOn = true;
Bitmap characters = nullptr;

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_EVENT_QUEUE* fallingQueue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;
ALLEGRO_EVENT fallingEvent;

class BitmapLoader* bitmaploader;

class Sprite* mario;
class MovingAnimator* walk;
class FrameRangeAnimator* jump;
bool lasttime_movedright = true;
bool not_moved = true;
bool jumped = false;
class FrameRangeAnimation *jump_anim = nullptr;
/*--------------------CLASSES---------------------------*/

//-------------Class Game----------------
void app::Game::Invoke(const Action& f) { if (f) f(); }
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

unsigned long app::currTime;

void SetGameTime() {
	app::currTime = GetSystemTime();
}

unsigned long GetGameTime() {
	return app::currTime;
}

void app::Game::MainLoopIteration(void) {
	SetGameTime();
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

const app::Game& app::App::GetGame(void) const { 
	return game; 
}

bool done() {
	return !closeWindowClicked;
}

void render() {
	circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);
	action_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);

	mario->GetCurrFilm()->DisplayFrame(BitmapGetScreen(), Point{ mario->GetBox().x, mario->GetBox().y }, mario->GetFrame());
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
			not_moved = true;
			if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP]) {
				if (jump_anim == nullptr && !mario->GetGravityHandler().isFalling()) {
					jump_anim = new FrameRangeAnimation("jump", 0, 17, 1, 0, -16, 15); //start, end, reps, dx, dy, delay

					jump_anim->SetChangeSpeed([](int &dx, int &dy, int frameNo) {
						//HERE CHAGNE DX AND DY DEPENDING ON FRAMENO
						int sumOfNumbers = 0;
						char maxTiles;

						if (mario->GetStateId() == RUNNING_STATE) {
							maxTiles = 5;
						}else {
							maxTiles = 4;
						}

						for (int i = 1; i <= jump_anim->GetEndFrame(); i++) sumOfNumbers += i;
						dy = -round((float)((jump_anim->GetEndFrame() - frameNo) * maxTiles * TILE_HEIGHT) / sumOfNumbers);
					});
					jump->Start(jump_anim, GetGameTime());
					mario->SetStateId(JUMPING_STATE);
					if (lasttime_movedright)
						mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_right"));
					else
						mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_left"));
				}
			}
			if (keys[ALLEGRO_KEY_S] || keys[ALLEGRO_KEY_DOWN]) {
				if (mario->GetBox().y + mario->GetBox().h < action_layer->GetViewWindow().h) {
					mario->Move(0, CHARACTER_MOVE_SPEED);
				}
			}

			if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
				if (lasttime_movedright) 
					mario->resetSpeed();
				else
					mario->incSpeed(GetGameTime());

				if (mario->GetBox().x > 0) {
					mario->Move(-mario->GetSpeed(), 0);
				}

				if (jump_anim == nullptr) {
					mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.walk_left"));
				}
				lasttime_movedright = false;
				not_moved = false;
			}
			if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
				if (mario->GetBox().x + mario->GetBox().w < action_layer->GetViewWindow().w) {
					if (lasttime_movedright)
						mario->incSpeed(GetGameTime());
					else
						mario->resetSpeed();
					mario->Move(mario->GetSpeed(), 0);
					int move_x = mario->GetSpeed();
					int move_y = 0;
					if (app::characterStaysInCenter(mario->GetBox(), &move_x)) {
						action_layer->ScrollWithBoundsCheck(&move_x, &move_y);
						circular_background->Scroll(move_x);
						mario->SetPos(mario->GetBox().x - move_x, mario->GetBox().y - move_y);
					}
					if (jump_anim == nullptr) {
						mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.walk_right"));
					}
					lasttime_movedright = true;
					not_moved = false;
				}
			}

			if (not_moved && jump_anim == nullptr) { //im not moving
				mario->SetFrame(0);
				mario->SetStateId(WALKING_STATE);
				mario->resetSpeed();
				if (lasttime_movedright)
					mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"));
				else
					mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_left"));
			}
		}
	}
	
}

void progress_animations() {
	AnimatorManager::GetSingleton().Progress(GetGameTime());
}

void physics() {
	int falling_dy;

	if (!al_is_event_queue_empty(fallingQueue)) {
		al_wait_for_event(fallingQueue, &fallingEvent);
		if (mario->GetGravityHandler().isFalling()) {
			mario->GetGravityHandler().increaseFallingTimer();
			falling_dy = (int)(mario->GetGravityHandler().getFallingTimer() / 4) * GRAVITY_G;
			mario->GetGravityHandler().checkFallingDistance(falling_dy, mario->GetBox());
			mario->Move(0, falling_dy); //gravity move down
		}
	}
}

void loadMap(string path) {
	app::ReadTextMap(action_layer, path);
}

//--------------------------------------

void loadSolidTiles(ALLEGRO_CONFIG* config, TileLayer *layer) {
	string text = "";
	vector<string> tiles;

	text = al_get_config_value(config, "tiles", "solid");
	tiles = splitString(text, " ");
	for (auto tile : tiles) {
		layer->insertSolid(atoi(tile.c_str()));
	}
	
}

void Sprite_MoveAction(Sprite* sprite, const MovingAnimation& anim) {
	sprite->Move(anim.GetDx(), anim.GetDy());
	sprite->NextFrame();
}

void FrameRange_Action(Sprite* sprite, Animator* animator, FrameRangeAnimation& anim) {
	auto* frameRangeAnimator = (FrameRangeAnimator*)animator;
	sprite->GetBox();
	if (frameRangeAnimator->GetCurrFrame() != anim.GetStartFrame() || frameRangeAnimator->GetCurrRep())
		sprite->Move(anim.GetDx(), anim.GetDy());

	sprite->SetFrame(frameRangeAnimator->GetCurrFrame());

	anim.ChangeSpeed(frameRangeAnimator->GetCurrFrame()); //changes Dx and Dy
}

void app::MainApp::Initialise(void) {
	SetGameTime();
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

	fallingQueue = al_create_event_queue();
	fallingTimer = al_create_timer(1.0 / 60);
	al_register_event_source(fallingQueue, al_get_timer_event_source(fallingTimer));
	al_start_timer(fallingTimer);

	//TODO delete this later we dont need it. Animation has one bitmaploader
	bitmaploader = new BitmapLoader();

	walk = new MovingAnimator();
	jump = new FrameRangeAnimator();

	AnimatorManager::GetSingleton().Register(walk);
	AnimatorManager::GetSingleton().Register(jump);
	walk->SetOnAction([](Animator* animator, const Animation& anim) {
		Sprite_MoveAction(mario, (const MovingAnimation&)anim);
	});

	jump->SetOnAction([](Animator* animator, const Animation& anim) {
		FrameRange_Action(mario, animator, (FrameRangeAnimation&)anim);
	});

	jump->SetOnStart([](Animator* animator) {
		mario->GetGravityHandler().setGravityAddicted(false);
	});
	jump->SetOnFinish([](Animator* animator) {
		delete jump_anim;
		jump_anim = nullptr;
		jump->Stop();
		AnimatorManager::GetSingleton().MarkAsSuspended(jump);
		mario->GetGravityHandler().setGravityAddicted(true);
		mario->GetGravityHandler().Check(mario->GetBox());
		mario->SetFrame(0);
	});
	
	walk->Start(new MovingAnimation("walk", 0, 0, 0, 80), GetGameTime());
}

string loadAllCharacters(const ALLEGRO_CONFIG* config) {

	return "Mario_small.walk_right:" + string(al_get_config_value(config, "Mario_small", "walk_right")) + '$'
		 + "Mario_small.walk_left:" + string(al_get_config_value(config, "Mario_small", "walk_left")) + '$'
		 + "Mario_small.stand_right:" + string(al_get_config_value(config, "Mario_small", "stand_right")) + '$'
		 + "Mario_small.stand_left:" + string(al_get_config_value(config, "Mario_small", "stand_left")) + '$'
		 + "Mario_small.jump_right:" + string(al_get_config_value(config, "Mario_small", "jump_right")) + '$'
		 + "Mario_small.jump_left:" + string(al_get_config_value(config, "Mario_small", "jump_left")) + '$'
		;
}

void app::MainApp::Load(void) {

	ALLEGRO_CONFIG* config = al_load_config_file(".\\UnitTests\\UnitTest3\\config.ini");
	assert(config != NULL);

	//load bitmaps, TODO we shouldnt have a bitmap loader at all. Animation film will handle this
	Bitmap tiles = bitmaploader->Load(al_get_config_value(config, "paths", "tiles_path"));
	characters = bitmaploader->Load(al_get_config_value(config, "paths", "characters_path"));

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
	game.SetPhysics(physics);
	game.SetProgressAnimations(progress_animations);

	loadSolidTiles(config, action_layer);
	action_layer->ComputeTileGridBlocks1();

	AnimationFilmHolder::GetInstance().LoadAll(loadAllCharacters(config), al_get_config_value(config, "paths", "characters_path"));
	mario = new Sprite(60, 430, AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"), "mario_small");
	mario->SetMover([](const Rect& pos, int* dx, int* dy) {
		Rect posOnGrid{
			pos.x + action_layer->GetViewWindow().x,
			pos.y + action_layer->GetViewWindow().y,
			pos.w,
			pos.h,
		};
		if (gridOn)
			action_layer->GetGrid()->FilterGridMotion(posOnGrid, dx, dy);
		mario->SetPos(pos.x + *dx, pos.y + *dy);
	});

	PrepareSpriteGravityHandler(action_layer->GetGrid(), mario);
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
	delete bitmaploader;
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
					layer->SetTile(x, y, total_tiles); //"create" an extra tile which is going to be identified as the trasnparent tile
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

int app::GetMapPixelWidth(void) {
	return widthInTiles * TILE_WIDTH > displayArea.w ? widthInTiles * TILE_WIDTH : displayArea.w;
}

int app::GetMapPixelHeight(void) {
	return heightInTiles * TILE_HEIGHT > displayArea.h ? heightInTiles * TILE_HEIGHT : displayArea.h;
}

bool app::characterStaysInCenter(Rect pos, int* dx) {
	return pos.x + pos.w/2 - *dx > displayArea.w/2;
}