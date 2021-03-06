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
bool disable_input = false;
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

class Sprite* mario; //replace mario with SpriteManager::GetSingleton().GetTypeList("mario").front()
class MovingAnimator* walk, *pipe_movement;
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
			}
		}
	);
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

void InitialiseGame(Game& game) {

	InstallPauseResumeHandler(game);

	game.SetDone(
		[](void) {
			return !closeWindowClicked;
		}
	);

	game.SetRender(
		[](void) {
			circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);
			action_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);

			for (auto sprite : SpriteManager::GetSingleton().GetDisplayList()) {
				sprite->Display(BitmapGetScreen());
			}
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
					cout << game.IsPaused() << endl;
					if (game.IsPaused())
						game.Resume();
					else
						game.Pause(GetGameTime());
					cout << game.IsPaused() << endl;
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

								if (keys[ALLEGRO_KEY_W])
									dy = -customRound((float)((jump_anim->GetEndFrame() - frameNo) * maxTiles * TILE_HEIGHT) / sumOfNumbers);
								else
									jump->Stop();
								});
							jump->Start(jump_anim, GetGameTime());
							//mario->SetStateId(JUMPING_STATE);
							if (lasttime_movedright)
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_right"));
							else
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.jump_left"));
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
								auto sprites = SpriteManager::GetSingleton().GetTypeList("pipe");
								for (auto sprite : sprites) { // move the sprites the opposite directions (f.e. pipes)
									sprite->Move(-move_x, 0);
									//if (sprite->GetBox().x + sprite->GetBox().w < 0) // if it is off the screen delete it
									//	SpriteManager::GetSingleton().Remove(sprite);
								}
								mario->Move(-move_x, -move_y);
								//mario->SetPos(mario->GetBox().x - move_x, mario->GetBox().y - move_y);
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
	);

	game.SetPhysics(
		[](void) {
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

void MoveScene(int new_screen_x, int new_screen_y, int new_mario_x, int new_mario_y) {
	Sprite* mario = SpriteManager::GetSingleton().GetTypeList("mario").front();
	mario->SetHasDirectMotion(true);
	mario->Move(new_mario_x - mario->GetBox().x, new_mario_y - mario->GetBox().y);
	mario->SetHasDirectMotion(false);
	//mario->UnconditionalMove(new_mario_x - mario->GetBox().x, new_mario_y - mario->GetBox().y);

	auto sprites = SpriteManager::GetSingleton().GetTypeList("pipe");
	for (auto sprite : sprites) { // move the sprites the opposite directions (f.e. pipes)
		sprite->Move(-(new_screen_x - action_layer->GetViewWindow().x), 0);
	}

	circular_background->Scroll(new_screen_x - action_layer->GetViewWindow().x);
	action_layer->SetViewWindow(Rect{ new_screen_x, new_screen_y, action_layer->GetViewWindow().w, action_layer->GetViewWindow().h });
}

void app::MainApp::Initialise(void) {
	SetGameTime();
	if (!al_init()) {
		std::cout << "ERROR: Could not init allegro\n";
		assert(false);
	}
	al_set_new_display_flags(ALLEGRO_WINDOWED);
	display = al_create_display(displayArea.w, displayArea.h);
	al_set_window_title(display, "Super Mario... CSD");
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

	pipe_movement->SetOnAction([](Animator* animator, const Animation& anim) {
		Sprite_MoveAction(mario, (const MovingAnimation&)anim);
	});
	pipe_movement->SetOnStart([](Animator* animator) {
		mario->SetHasDirectMotion(true);
		mario->GetGravityHandler().setGravityAddicted(false);
		mario->SetFrame(0);
		if (lasttime_movedright)
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"));
		else
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_left"));
		mario->GetGravityHandler().SetFalling(false);
		AnimatorManager::GetSingleton().MarkAsSuspended(walk);
		if (jump->IsAlive()) {
			jump->Stop();
			AnimatorManager::GetSingleton().MarkAsSuspended(jump);
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

Sprite * LoadPipeCollision(Sprite * mario, string pipes) {
	vector<string> coordinates = splitString(pipes.substr(1, pipes.length()), " ");
	int x = atoi(coordinates[0].c_str());
	int y = atoi(coordinates[1].c_str());
	int new_screen_x = atoi(coordinates[2].c_str());
	int new_screen_y = atoi(coordinates[3].c_str());
	int new_mario_x = atoi(coordinates[4].c_str());
	int new_mario_y = atoi(coordinates[5].c_str());


	Sprite* tmp = nullptr;
	switch (pipes.at(0)) {
	case 'u':
		tmp = new Sprite(x, y, AnimationFilmHolder::GetInstance().GetFilm("Pipe.up"), "pipe");
		CollisionChecker::GetSingleton().Register(mario, tmp, [mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Sprite* s1, Sprite* s2) {

			int s1_y1 = ((const BoundingBox*)(s1->GetBoundingArea()))->getY1();
			int s2_y2 = ((const BoundingBox*)(s2->GetBoundingArea()))->getY2();
			int s1_x1 = ((const BoundingBox*)(s1->GetBoundingArea()))->getX1();
			int s1_x2 = ((const BoundingBox*)(s1->GetBoundingArea()))->getX2();
			int s2_x1 = ((const BoundingBox*)(s2->GetBoundingArea()))->getX1();
			int s2_x2 = ((const BoundingBox*)(s2->GetBoundingArea()))->getX2();

			if (!(s2_y2 < s1_y1) && (s1_x1 >= s2_x1) && (s1_x2 <= s2_x2) && keys[ALLEGRO_KEY_S]) { //if mario on top of the pipe
				//PLAY ANIMATION HERE

				if (&pipe_movement->GetAnim() != nullptr) // if animation is already playing
					return;
				pipe_movement->Start(new MovingAnimation("pipe.up", 32, 0, 1, 30), GetGameTime());

				pipe_movement->SetOnFinish([mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Animator* animator) {
					pipe_movement->deleteCurrAnimation();
					mario->SetHasDirectMotion(false);
					animator->Stop();
					AnimatorManager::GetSingleton().MarkAsSuspended(pipe_movement);
					mario->SetFrame(0);
					mario->GetGravityHandler().setGravityAddicted(true);
					MoveScene(new_screen_x, new_screen_y, new_mario_x, new_mario_y);
					AnimatorManager::GetSingleton().MarkAsRunning(walk);
					disable_input = false;
				});
				
			}
		});
		break;
	case 'd':
		tmp = new Sprite(x, y, AnimationFilmHolder::GetInstance().GetFilm("Pipe.down"), "pipe");
		CollisionChecker::GetSingleton().Register(mario, tmp, [mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Sprite* s1, Sprite* s2) {

			int s1_y2 = ((const BoundingBox*)(s1->GetBoundingArea()))->getY2();
			int s2_y1 = ((const BoundingBox*)(s2->GetBoundingArea()))->getY1();

			if (!(s1_y2 < s2_y1) && keys[ALLEGRO_KEY_W]) { //if mario bellow the pipe

				if (&pipe_movement->GetAnim() != nullptr) // if animation is already playing
					return; 
				pipe_movement->Start(new MovingAnimation("pipe.down", 32, 0, -1, 30), GetGameTime());

				pipe_movement->SetOnFinish([mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Animator* animator) {
					pipe_movement->deleteCurrAnimation();
					mario->SetHasDirectMotion(false);
					animator->Stop();
					AnimatorManager::GetSingleton().MarkAsSuspended(pipe_movement);
					mario->SetFrame(0);
					mario->GetGravityHandler().setGravityAddicted(true);
					MoveScene(new_screen_x, new_screen_y, new_mario_x, new_mario_y);
					AnimatorManager::GetSingleton().MarkAsRunning(walk);
					disable_input = false;
				});
			}
		});
		break;
	case 'l':
		tmp = new Sprite(x, y, AnimationFilmHolder::GetInstance().GetFilm("Pipe.left"), "pipe");
		CollisionChecker::GetSingleton().Register(mario, tmp, [mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Sprite* s1, Sprite* s2) {

			int s1_x1 = ((const BoundingBox*)(s1->GetBoundingArea()))->getX2();
			int s2_x2 = ((const BoundingBox*)(s2->GetBoundingArea()))->getX1();

			if (!(s2_x2 < s1_x1) && keys[ALLEGRO_KEY_D]) { //if mario on left of the pipe

				if (&pipe_movement->GetAnim() != nullptr) // if animation is already playing
					return;
				pipe_movement->Start(new MovingAnimation("pipe.left", 32, 1, 0, 30), GetGameTime());

				pipe_movement->SetOnFinish([mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Animator* animator) {
					pipe_movement->deleteCurrAnimation();
					mario->SetHasDirectMotion(false);
					animator->Stop();
					AnimatorManager::GetSingleton().MarkAsSuspended(pipe_movement);
					mario->SetFrame(0);
					mario->GetGravityHandler().setGravityAddicted(true);
					MoveScene(new_screen_x, new_screen_y, new_mario_x, new_mario_y);
					AnimatorManager::GetSingleton().MarkAsRunning(walk);
					disable_input = false;
				});
			}
		});
		break;
	case 'r':
		tmp = new Sprite(x, y, AnimationFilmHolder::GetInstance().GetFilm("Pipe.right"), "pipe");
		CollisionChecker::GetSingleton().Register(mario, tmp, [mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Sprite* s1, Sprite* s2) {

			int s1_y1 = ((const BoundingBox*)(s1->GetBoundingArea()))->getX2();
			int s2_y2 = ((const BoundingBox*)(s2->GetBoundingArea()))->getX1();

			if (!(s1_y1 < s2_y2) && keys[ALLEGRO_KEY_A]) { //if mario on right of the pipe

				if (&pipe_movement->GetAnim() != nullptr) // if animation is already playing
					return;

				pipe_movement->Start(new MovingAnimation("pipe.right", 32, -1, 0, 30), GetGameTime());

				pipe_movement->SetOnFinish([mario, new_screen_x, new_screen_y, new_mario_x, new_mario_y](Animator* animator) {
					pipe_movement->deleteCurrAnimation();
					mario->SetHasDirectMotion(false);
					animator->Stop();
					AnimatorManager::GetSingleton().MarkAsSuspended(pipe_movement);
					mario->SetFrame(0);
					mario->GetGravityHandler().setGravityAddicted(true);
					MoveScene(new_screen_x, new_screen_y, new_mario_x, new_mario_y);
					AnimatorManager::GetSingleton().MarkAsRunning(walk);
					disable_input = false;
				});
			}
		});
		break;
	}

	assert(tmp);
	tmp->SetHasDirectMotion(true);
	tmp->GetGravityHandler().setGravityAddicted(false);
	tmp->SetZorder(1);
	tmp->SetBoundingArea(new BoundingBox(tmp->GetBox().x, tmp->GetBox().y, tmp->GetBox().x + tmp->GetBox().w, tmp->GetBox().y + tmp->GetBox().h));

	return tmp;
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

	loadSolidTiles(config, action_layer);
	action_layer->ComputeTileGridBlocks1();

	AnimationFilmHolder::GetInstance().LoadAll(loadAllCharacters(config), al_get_config_value(config, "paths", "characters_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllPipes(config), al_get_config_value(config, "paths", "tiles_path"));

	mario = new Sprite(60, 430, AnimationFilmHolder::GetInstance().GetFilm("Mario_small.stand_right"), "mario");
	SpriteManager::GetSingleton().Add(mario);

	//create all pipe sprites and add collisions
	for (auto pipes : splitString(al_get_config_value(config, "pipes", "pipe_locations"), ",")) {

		Sprite *pipe = LoadPipeCollision(mario, pipes);
		SpriteManager::GetSingleton().Add(pipe);
		
	}
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

		if (old_dx > *dx && old_dx > 0 || old_dx < *dx && old_dx < 0)
			mario->SetStateId(IDLE_STATE);
	});

	mario->SetBoundingArea(new BoundingBox(mario->GetBox().x, mario->GetBox().y, mario->GetBox().x + mario->GetBox().w, mario->GetBox().y + mario->GetBox().h));

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