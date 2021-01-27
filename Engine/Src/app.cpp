#include "app.h"
#include "Bitmap.h"
#include "Utilities.h"
#include "Animation.h"
#include "Animator.h"
#include "createNPCs.h"
#include "Sprite.h"

using namespace std;

//--------------------GLOBAL VARS-----------------------
class TileLayer* action_layer;
TileLayer* underground_layer;
class CircularBackground* circular_background;

Rect displayArea = Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
int widthInTiles = 0, heightInTiles = 0;
unsigned int total_tiles;
bool closeWindowClicked = false;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
ALLEGRO_TIMER* fallingTimer;
ALLEGRO_TIMER* aiTimer;
bool gridOn = true;
bool disable_input = false;
Bitmap characters = nullptr;
Bitmap npcs = nullptr;

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_EVENT_QUEUE* fallingQueue;
ALLEGRO_EVENT_QUEUE* aiQueue;
bool scrollEnabled = false;
int mouse_x = 0, mouse_y = 0, prev_mouse_x = 0, prev_mouse_y = 0;
ALLEGRO_MOUSE_STATE mouse_state;
ALLEGRO_EVENT event;

class BitmapLoader* bitmaploader;

class Sprite* mario; //replace mario with SpriteManager::GetSingleton().GetTypeList("mario").front()
class MovingAnimator* walk, * pipe_movement;
class FrameRangeAnimator* jump;
bool not_moved = true;
bool jumped = false;
class FrameRangeAnimation* jump_anim = nullptr;

std::unordered_set <Sprite*> shells;

Bitmap liveIcon = nullptr;

ALLEGRO_FONT* font;
ALLEGRO_FONT* paused_font;
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
	Input();
	if (!IsPaused()) {
		Render();
		ProgressAnimations();
		AI();
		Physics();
		CollisionChecking();
		CommitDestructions();
		UserCode(); // hook for custom code at end
	}
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

void InstallPauseResumeHandler(Game& game) {
	game.SetOnPauseResume(
		[&game](void) {
			if (!game.IsPaused()) { // just resumed
				AnimatorManager::GetSingleton().TimeShift(
					GetGameTime() - game.GetPauseTime()
				);
				al_flush_event_queue(fallingQueue);
				al_flush_event_queue(aiQueue);
			}
			else {
				al_draw_text(paused_font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, action_layer->GetViewWindow().h / 2, ALLEGRO_ALIGN_CENTER, "Paused");
				al_flip_display();
			}
		}
	);
}

void loadMap(TileLayer *layer, string path) {
	app::ReadTextMap(layer, path);
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

void InitialiseGame(Game& game) {

	InstallPauseResumeHandler(game);

	game.SetDone(
		[](void) {
			return !closeWindowClicked;
		}
	);

	game.SetRender(
		[game](void) {
			circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);
			underground_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);
			action_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);

			for(int i = 0; i < mario->getLives(); i++)
				al_draw_scaled_bitmap(liveIcon, 0, 0, 16, 16, 50 + (i * 15), 20, 14, 14, 0);

			for (auto sprite : SpriteManager::GetSingleton().GetDisplayList()) {
				sprite->Display(BitmapGetScreen());
			}
			
			al_draw_text(font, al_map_rgb(0, 0, 0), 30, 18, ALLEGRO_ALIGN_CENTER, "Lives: ");

			if (game.IsPaused())
				al_draw_text(paused_font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, action_layer->GetViewWindow().h / 2, ALLEGRO_ALIGN_CENTER, "Paused");

			al_flip_display();
		}
	);

	game.SetInput(
		[&game] {
			if (!al_is_event_queue_empty(queue)) {
				al_wait_for_event(queue, &event);

				if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_TILDE) {
					gridOn = !gridOn;
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_P) {
					if (game.IsPaused())
						game.Resume();
					else
						game.Pause(GetGameTime());
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_K) {
					mario->addLife();
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_L) {
					mario->loseLife();
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
					if (game.IsPaused() || disable_input)
						return;
					not_moved = true;
					if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP]) {
						if (jump_anim == nullptr && !mario->GetGravityHandler().isFalling()) {
							jump_anim = new FrameRangeAnimation("jump", 0, 17, 1, 0, -16, 15); //start, end, reps, dx, dy, delay

							jump_anim->SetChangeSpeed([](int& dx, int& dy, int frameNo) {
								//HERE CHAGNE DX AND DY DEPENDING ON FRAMENO
								int sumOfNumbers = 0;
								char maxTiles;

								if (mario->GetStateId() == RUNNING_STATE) {
									maxTiles = 5;
								}
								else {
									maxTiles = 4;
								}

								for (int i = 1; i <= jump_anim->GetEndFrame(); i++) sumOfNumbers += i;

								if (keys[ALLEGRO_KEY_W] || keys[ALLEGRO_KEY_UP])
									dy = -customRound((float)((jump_anim->GetEndFrame() - frameNo) * maxTiles * TILE_HEIGHT) / sumOfNumbers);
								else
									jump->Stop();
								});
							jump->Start(jump_anim, GetGameTime());
							if (mario->lastMovedRight)
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_right"));
							else
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_left"));
						}
					}

					if (keys[ALLEGRO_KEY_A] || keys[ALLEGRO_KEY_LEFT]) {
						if (mario->lastMovedRight)
							mario->resetSpeed();
						else
							mario->incSpeed(GetGameTime());

						if (mario->GetBox().x > 0) {
							mario->Move(-mario->GetSpeed(), 0);
						}

						if (jump_anim == nullptr) {
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.walk_left"));
						}
						mario->lastMovedRight = false;
						not_moved = false;
					}
					if (keys[ALLEGRO_KEY_D] || keys[ALLEGRO_KEY_RIGHT]) {
						if (mario->GetBox().x + mario->GetBox().w < action_layer->GetViewWindow().w) {
							if (mario->lastMovedRight)
								mario->incSpeed(GetGameTime());
							else
								mario->resetSpeed();
							mario->Move(mario->GetSpeed(), 0);
							int move_x = mario->GetSpeed();
							int move_y = 0;
							if (app::characterStaysInCenter(mario->GetBox(), &move_x)) {
								underground_layer->ScrollWithBoundsCheck(&move_x, &move_y);
								action_layer->ScrollWithBoundsCheck(&move_x, &move_y);
								circular_background->Scroll(move_x);
								for (auto sprite : SpriteManager::GetSingleton().GetTypeList("pipe")) { // move the sprites the opposite directions (f.e. pipes)
									sprite->Move(-move_x, 0);
									//if (sprite->GetBox().x + sprite->GetBox().w < 0) // if it is off the screen delete it
									//	SpriteManager::GetSingleton().Remove(sprite);
								}
								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("goomba")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}

								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("green_koopa_troopa")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}

								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("red_koopa_troopa")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}
								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("piranha_plant")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}

								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("block")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}

								if (move_x != 0) {
									for (auto sprite : SpriteManager::GetSingleton().GetTypeList("brick")) { // move the sprites the opposite directions (f.e. pipes)
										sprite->Move(-move_x, 0);
									}
								}

								mario->Move(-move_x, -move_y);
								//mario->SetPos(mario->GetBox().x - move_x, mario->GetBox().y - move_y);
							}
							if (jump_anim == nullptr) {
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.walk_right"));
							}
							mario->lastMovedRight = true;
							not_moved = false;
						}
					}

					if (not_moved && jump_anim == nullptr) { //im not moving
						mario->SetFrame(0);
						mario->SetStateId(WALKING_STATE);
						mario->resetSpeed();
						if (mario->lastMovedRight)
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"));
						else
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_left"));
					}
				}
			}
		}
	);

	game.SetPhysics(
		[](void) {
			int falling_dy;

			if (!al_is_event_queue_empty(fallingQueue)) {
				al_wait_for_event(fallingQueue, &event);
				for (auto sprite : SpriteManager::GetSingleton().GetDisplayList()) {
					if (sprite->GetGravityHandler().isFalling()) {
						sprite->GetGravityHandler().increaseFallingTimer();
						falling_dy = (int)(sprite->GetGravityHandler().getFallingTimer() / 4) * GRAVITY_G;
						sprite->GetGravityHandler().checkFallingDistance(falling_dy, sprite->GetBox());
						sprite->Move(0, falling_dy); //gravity move down
					}
				}
			}
		}
	);

	game.SetAI(
		[](void) {
			vector<Sprite*> toBeDestroyed;

			if (!al_is_event_queue_empty(aiQueue)) {
				al_wait_for_event(aiQueue, &event);
				if (!SpriteManager::GetSingleton().GetTypeList("goomba").empty()) {
					for (auto goomba : SpriteManager::GetSingleton().GetTypeList("goomba")) {
						if (goomba->GetFormStateId() == SMASHED) {
							//if smashed do nothing (dont move it)
						}
						else if (goomba->GetFormStateId() == DELETE) {
							goomba->SetVisibility(false);
							toBeDestroyed.push_back(goomba);
						}
						else {
							if (goomba->lastMovedRight)
								goomba->Move(ENEMIES_MOVE_SPEED, 0);
							else
								goomba->Move(-ENEMIES_MOVE_SPEED, 0);

						}
					}
				}
				if (!SpriteManager::GetSingleton().GetTypeList("green_koopa_troopa").empty()) {
					for (auto koopa_troopa : SpriteManager::GetSingleton().GetTypeList("green_koopa_troopa")) {
						if (koopa_troopa->GetFormStateId() == DELETE) {
							koopa_troopa->SetVisibility(false);
							toBeDestroyed.push_back(koopa_troopa);
						}
						else {
							if (koopa_troopa->GetStateId() == WALKING_STATE) {
								int speed = ENEMIES_MOVE_SPEED;
								if (koopa_troopa->GetFormStateId() == SMASHED)
									speed = SHELL_SPEED;
								if (koopa_troopa->lastMovedRight)
									koopa_troopa->Move(speed, 0);
								else
									koopa_troopa->Move(-speed, 0);
							}
						}
					}
				}
				if (!SpriteManager::GetSingleton().GetTypeList("red_koopa_troopa").empty()) {
					for (auto koopa_troopa : SpriteManager::GetSingleton().GetTypeList("red_koopa_troopa")) {
						if (koopa_troopa->GetFormStateId() == DELETE) {
							koopa_troopa->SetVisibility(false);
							toBeDestroyed.push_back(koopa_troopa);
						}
						else {
							if (koopa_troopa->GetStateId() == WALKING_STATE) {
								int speed = ENEMIES_MOVE_SPEED;
								if (koopa_troopa->GetFormStateId() == SMASHED)
									speed = SHELL_SPEED;
								if (koopa_troopa->lastMovedRight)
									koopa_troopa->Move(speed, 0);
								else
									koopa_troopa->Move(-speed, 0);
							}
						}
					}
				}
				if (!SpriteManager::GetSingleton().GetTypeList("piranha_plant").empty()) {
					for (auto plant : SpriteManager::GetSingleton().GetTypeList("piranha_plant")) {
						if (plant->GetFormStateId() == DELETE) {
							plant->SetVisibility(false);
							toBeDestroyed.push_back(plant);
						}
					}
				}
				if (!SpriteManager::GetSingleton().GetTypeList("block").empty()) {
					for (auto block : SpriteManager::GetSingleton().GetTypeList("block")) {
						if (block->GetFormStateId() == MOVED_BLOCK) {
							block->Move(0, 4);
							block->SetFormStateId(BLOCK);
						}
					}
				}
				if (!SpriteManager::GetSingleton().GetTypeList("brick").empty()) {
					for (auto brick : SpriteManager::GetSingleton().GetTypeList("brick")) {
						if (brick->GetFormStateId() == MOVED_BLOCK) {
							brick->Move(0, 4);
							brick->SetFormStateId(BRICK);
						}
						else if (brick->GetFormStateId() == SMASHED) {
							toBeDestroyed.push_back(brick);
						}
					}
				}
				for (auto sprite : toBeDestroyed) {
					SpriteManager::GetSingleton().Remove(sprite);
				}
				toBeDestroyed.clear();
			}
		}
	);

	game.SetCollisionCheck(
		[](void) {
			CollisionChecker::GetSingletonConst().Check();
		}
	);

	game.SetProgressAnimations(
		[](void) {
			AnimatorManager::GetSingleton().Progress(GetGameTime());
		}
	);
}

void app::MainApp::Initialise(void) {
	SetGameTime();
	if (!al_init()) {
		std::cout << "ERROR: Could not init allegro\n";
		assert(false);
	}
	al_init_image_addon();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	al_set_new_display_flags(ALLEGRO_WINDOWED);
	display = al_create_display(displayArea.w, displayArea.h);
	al_set_window_title(display, "Super Mario... CSD");
	queue = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));

	timer = al_create_timer(1.0 / 60);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);

	fallingQueue = al_create_event_queue();
	fallingTimer = al_create_timer(1.0 / 60);
	al_register_event_source(fallingQueue, al_get_timer_event_source(fallingTimer));
	al_start_timer(fallingTimer);

	aiQueue= al_create_event_queue();
	aiTimer = al_create_timer(1.0 / 60);
	al_register_event_source(aiQueue, al_get_timer_event_source(aiTimer));
	al_start_timer(aiTimer);

	font = al_load_font(".\\Engine\\Media\\game_font.ttf", 20, NULL);
	paused_font = al_load_font(".\\Engine\\Media\\game_font.ttf", 40, NULL);

	InitialiseGame(game);

	//TODO delete this later we dont need it. Animation has one bitmaploader
	bitmaploader = new BitmapLoader();

	walk = new MovingAnimator();
	jump = new FrameRangeAnimator();
	pipe_movement = new MovingAnimator();

	AnimatorManager::GetSingleton().Register(walk);
	AnimatorManager::GetSingleton().Register(jump);
	AnimatorManager::GetSingleton().Register(pipe_movement);
	walk->SetOnAction([](Animator* animator, const Animation& anim) {
		Sprite_MoveAction(mario, (const MovingAnimation&)anim);
	});

	jump->SetOnAction([](Animator* animator, const Animation& anim) {
		FrameRange_Action(mario, animator, (FrameRangeAnimation&)anim);
	});

	jump->SetOnStart([](Animator* animator) {
		mario->GetGravityHandler().SetFalling(false);
		mario->GetGravityHandler().setGravityAddicted(false);
	});
	jump->SetOnFinish([](Animator* animator) {
		delete jump_anim;
		jump_anim = nullptr;
		jump->Stop();
		mario->GetGravityHandler().setGravityAddicted(true);
		mario->GetGravityHandler().Check(mario->GetBox());
		mario->SetFrame(0);
	});

	pipe_movement->SetOnAction([](Animator* animator, const Animation& anim) {
		Sprite_MoveAction(mario, (const MovingAnimation&)anim);
	});
	pipe_movement->SetOnStart([](Animator* animator) {
		mario->SetHasDirectMotion(true);
		mario->GetGravityHandler().setGravityAddicted(false);
		mario->GetGravityHandler().SetFalling(false);
		mario->SetFrame(0);
		if (mario->lastMovedRight)
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"));
		else
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_left"));
		AnimatorManager::GetSingleton().MarkAsSuspended(walk);

		if (jump->IsAlive()) {
			jump->Stop();
		}
		disable_input = true;
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

string loadAllPipes(const ALLEGRO_CONFIG* config) {

	return "Pipe.up:" + string(al_get_config_value(config, "pipes", "pipe_up")) + '$'
		+ "Pipe.right:" + string(al_get_config_value(config, "pipes", "pipe_right")) + '$'
		+ "Pipe.left:" + string(al_get_config_value(config, "pipes", "pipe_left")) + '$'
		;
}

string loadAllEnemies(const ALLEGRO_CONFIG* config) {

	return "enemies.goomba:" + string(al_get_config_value(config, "enemies", "goomba")) + '$'
		+ "enemies.goomba_smashed:" + string(al_get_config_value(config, "enemies", "goomba_smashed")) + '$'
		+ "enemies.green_koopa_troopa_right:" + string(al_get_config_value(config, "enemies", "green_koopa_troopa_right")) + '$'
		+ "enemies.green_koopa_troopa_left:" + string(al_get_config_value(config, "enemies", "green_koopa_troopa_left")) + '$'
		+ "enemies.green_koopa_troopa_shell:" + string(al_get_config_value(config, "enemies", "green_koopa_troopa_shell")) + '$'
		+ "enemies.red_koopa_troopa_right:" + string(al_get_config_value(config, "enemies", "red_koopa_troopa_right")) + '$'
		+ "enemies.red_koopa_troopa_left:" + string(al_get_config_value(config, "enemies", "red_koopa_troopa_left")) + '$'
		+ "enemies.red_koopa_troopa_shell:" + string(al_get_config_value(config, "enemies", "red_koopa_troopa_shell")) + '$'
		+ "enemies.piranha_plant:" + string(al_get_config_value(config, "enemies", "piranha_plant")) + '$'
		;
}

string loadAllBlocks(const ALLEGRO_CONFIG* config) {

	return "blocks.brick:" + string(al_get_config_value(config, "blocks", "brick")) + '$'
		+ "blocks.block:" + string(al_get_config_value(config, "blocks", "block")) + '$'
		;
}

void app::MainApp::Load(void) {
	ALLEGRO_CONFIG* config = al_load_config_file(".\\Engine\\config.ini");
	assert(config != NULL);

	//load bitmaps, TODO we shouldnt have a bitmap loader at all. Animation film will handle this
	Bitmap tiles = bitmaploader->Load(al_get_config_value(config, "paths", "tiles_path"));
	Bitmap bg_tiles = bitmaploader->Load(al_get_config_value(config, "paths", "bg_tiles_path"));
	characters = bitmaploader->Load(al_get_config_value(config, "paths", "characters_path"));
	npcs = bitmaploader->Load(al_get_config_value(config, "paths", "npcs_path"));
	assert(npcs);

	action_layer = new TileLayer(MAX_HEIGHT, MAX_WIDTH, tiles);
	action_layer->Allocate();

	int tilesw = DIV_TILE_WIDTH(BitmapGetWidth(tiles)); //tileset width
	int tilesh = DIV_TILE_HEIGHT(BitmapGetHeight(tiles)); //tileset height
	total_tiles = tilesw * tilesh;

	action_layer->InitCaching(tilesw, tilesh);
	loadMap(action_layer, al_get_config_value(config, "paths", "action_layer_path"));

	underground_layer = new TileLayer(MAX_HEIGHT, MAX_WIDTH, tiles);
	underground_layer->Allocate();
	underground_layer->InitCaching(tilesw, tilesh);
	loadMap(underground_layer, al_get_config_value(config, "paths", "underground_layer_path"));

	circular_background = new CircularBackground(bg_tiles, al_get_config_value(config, "paths", "circular_backround_path"));

	loadSolidTiles(config, action_layer);
	action_layer->ComputeTileGridBlocks1();

	AnimationFilmHolder::GetInstance().LoadAll(loadAllCharacters(config), al_get_config_value(config, "paths", "characters_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllPipes(config), al_get_config_value(config, "paths", "tiles_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllEnemies(config), al_get_config_value(config, "paths", "enemies_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllBlocks(config), al_get_config_value(config, "paths", "npcs_path"));

	liveIcon = SubBitmapCreate(BitmapLoad(al_get_config_value(config, "paths", "characters_path")), {127, 60, 16, 16});

	vector<string> coordinates = splitString(al_get_config_value(config, "potitions", "start"), " ");
	mario = new Sprite(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()), AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"), "mario");
	SpriteManager::GetSingleton().Add(mario);

	mario->SetMover([](const Rect& pos, int* dx, int* dy) {
		int old_dx = *dx;
		Rect posOnGrid{
			pos.x + action_layer->GetViewWindow().x,
			pos.y + action_layer->GetViewWindow().y,
			pos.w,
			pos.h,
		};
		if (gridOn)
			action_layer->GetGrid()->FilterGridMotion(posOnGrid, dx, dy);
		mario->SetPos(pos.x + *dx, pos.y + *dy);

		if (old_dx > * dx && old_dx > 0 || old_dx < *dx && old_dx < 0) {
			mario->SetStateId(IDLE_STATE);
		}
		});

	mario->SetBoundingArea(new BoundingBox(mario->GetBox().x, mario->GetBox().y, mario->GetBox().x + mario->GetBox().w, mario->GetBox().y + mario->GetBox().h));
	mario->SetFormStateId(SMALL_MARIO);
	mario->setLives(3);

	PrepareSpriteGravityHandler(action_layer->GetGrid(), mario);

	vector<string> locations;

	//create a demo goomba
	locations = splitString(al_get_config_value(config, "emenies_positions", "goomba"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_enemy_goomba(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}

	//create a demo Koopa Troopa 
	locations = splitString(al_get_config_value(config, "emenies_positions", "green_koopa_troopa"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_enemy_green_koopa_troopa(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}


	/*--------------------create a demo Red Koopa Troopa --------------------------
	* Exactly the same as the green koopa but the mover function is changed. Only this!*/

	locations = splitString(al_get_config_value(config, "emenies_positions", "red_koopa_troopa"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_enemy_red_koopa_troopa(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}

	/*---------------------------Create demo piranha plant------------------------*/
	locations = splitString(al_get_config_value(config, "emenies_positions", "piranha_plant"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_enemy_piranha_plant(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}

	//create all pipe sprites and add collisions
	for (auto pipes : splitString(al_get_config_value(config, "pipes", "pipe_locations"), ",")) {

		Sprite *pipe = LoadPipeCollision(mario, pipes);
		pipe->SetFormStateId(PIPE);
		SpriteManager::GetSingleton().Add(pipe);
		
	}

	for (unsigned int i = 0; i < action_layer->GetMapWidth(); i++) {
		for (unsigned int j = 0; j < action_layer->GetMapHeight(); j++) {
			// replace each brick index with sprite
			if (action_layer->GetTile(j, i) == 0) {
				create_block_sprite(MUL_TILE_WIDTH(i), MUL_TILE_HEIGHT(j));
			}

			// replace each block index with sprite
			if (action_layer->GetTile(j, i) == 4) {
				create_brick_sprite(MUL_TILE_WIDTH(i), MUL_TILE_HEIGHT(j));
			}
		}
	}
}

void app::App::Run(void) {
	game.MainLoop();
}

void app::MainApp::Clear(void) {
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(action_layer->GetBitmap());
	//al_destroy_bitmap(underground_layer->GetBitmap());
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
		layer->SetMapDims(x, y);
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

void app::Game::Pause(uint64_t t) {
	isPaused = true; pauseTime = t; Invoke(pauseResume);
}

void app::Game::Resume(void) {
	isPaused = false; Invoke(pauseResume); pauseTime = 0;
}

bool app::Game::IsPaused(void) const {
	return isPaused;
}

uint64_t app::Game::GetPauseTime(void) const {
	return pauseTime;
}