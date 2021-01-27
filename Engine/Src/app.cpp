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
class CircularBackground* menu_circular_background;

Rect displayArea = Rect{ 0, 0, DISP_AREA_X, DISP_AREA_Y };
int widthInTiles = 0, heightInTiles = 0;
unsigned int total_tiles;
bool closeWindowClicked = false;
bool keys[ALLEGRO_KEY_MAX] = { 0 };
ALLEGRO_TIMER* timer;
ALLEGRO_TIMER* fallingTimer;
ALLEGRO_TIMER* aiTimer;
ALLEGRO_TIMER* blockTimer;
bool gridOn = true;
bool disable_input = false;
Bitmap characters = nullptr;
Bitmap npcs = nullptr;

ALLEGRO_DISPLAY* display;
ALLEGRO_EVENT_QUEUE* queue;
ALLEGRO_EVENT_QUEUE* fallingQueue;
ALLEGRO_EVENT_QUEUE* aiQueue;
ALLEGRO_EVENT_QUEUE* blockQueue;
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
Bitmap coinIcon = nullptr;

ALLEGRO_FONT* font;
ALLEGRO_FONT* paused_font;
ALLEGRO_FONT* tittle_font;
ALLEGRO_FONT* tittle_font_smaller;
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
		[&game](void) {
			circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);
			underground_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);
			action_layer->TileTerrainDisplay(al_get_backbuffer(display), displayArea);

			for (auto sprite : SpriteManager::GetSingleton().GetDisplayList()) {
				sprite->Display(BitmapGetScreen());
			}
			
			al_draw_text(font, al_map_rgb(255, 255, 255), 30, 18, ALLEGRO_ALIGN_CENTER, "Lives: ");
			if (game.getLives() < 6) {
				for (int i = 0; i < game.getLives(); i++)
					al_draw_scaled_bitmap(liveIcon, 0, 0, 16, 16, 50 + (i * 15), 20, 14, 14, 0);
			}
			else {
				al_draw_scaled_bitmap(liveIcon, 0, 0, 16, 16, 50, 20, 14, 14, 0);
				al_draw_text(font, al_map_rgb(255, 255, 255), 70, 19, ALLEGRO_ALIGN_LEFT, ("x" + to_string(game.getLives())).c_str());
			}

			al_draw_text(font, al_map_rgb(255, 255, 255), 290, 18, ALLEGRO_ALIGN_CENTER, "Coins: ");
			al_draw_scaled_bitmap(coinIcon, 0, 0, 16, 16, 310, 18, 16, 16, 0);
			al_draw_text(font, al_map_rgb(255, 255, 255), 325, 19, ALLEGRO_ALIGN_LEFT, ("x" + to_string(game.getCoins())).c_str());
			
			al_draw_text(font, al_map_rgb(255, 255, 255), 525, 18, ALLEGRO_ALIGN_CENTER, "Score: ");
			al_draw_text(font, al_map_rgb(255, 255, 255), 550, 19, ALLEGRO_ALIGN_LEFT, standarizeSize(to_string(game.getPoints()), 8).c_str());

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
					game.addLife();
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_L) {
					game.loseLife();
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_M) {
					game.addCoin();
				}
				else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_N) {
					game.addPoints(100);
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
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".jump_right"));
							else
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".jump_left"));
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
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".walk_left"));
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

							int move_x = mario->GetSpeed();
							int move_y = 0;
							mario->Move(move_x, 0);
							if (app::characterStaysInCenter(mario->GetBox(), &move_x)) {
								underground_layer->ScrollWithBoundsCheck(&move_x, &move_y);
								action_layer->ScrollWithBoundsCheck(&move_x, &move_y);
								circular_background->Scroll(move_x);

								auto sprites = SpriteManager::GetSingleton().GetDisplayList();
								sprites.remove(mario);

								for (auto sprite : SpriteManager::GetSingleton().GetTypeList("red_koopa_troopa")) { 
									sprite->SetHasDirectMotion(true);
									sprite->Move(-move_x, 0);
									sprite->SetHasDirectMotion(false);
									sprites.remove(sprite);
								}

								if (move_x != 0) {	// move the sprites the opposite directions (f.e. pipes)
									for (auto sprite : sprites) {
										sprite->Move(-move_x, 0);
									}
								}

								mario->Move(-move_x, -move_y);
								//mario->SetPos(mario->GetBox().x - move_x, mario->GetBox().y - move_y);
							}
							if (jump_anim == nullptr) {
								mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".walk_right"));
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
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".stand_right"));
						else
							mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".stand_left"));
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
		[&game](void) {
			vector<Sprite*> toBeDestroyed;
			vector<Sprite*> toBeDestroyedByBlock;

			if (!al_is_event_queue_empty(aiQueue)) {
				al_wait_for_event(aiQueue, &event);
				std::vector<std::string> npcs_types = { "goomba", "green_koopa_troopa", "red_koopa_troopa" };
				for(std::string enemie_type : npcs_types)
					for (auto enemie : SpriteManager::GetSingleton().GetTypeList(enemie_type)) {
						if (enemie_type == "goomba" && enemie->GetFormStateId() == SMASHED) {
							//if smashed do nothing (dont move it)
						}
						else if (enemie->GetFormStateId() == DELETE
							|| (enemie_type != "red_koopa_troopa" && enemie->GetBox().y > (action_layer->GetViewWindow().y + action_layer->GetViewWindow().h))) {
							enemie->SetVisibility(false);
							toBeDestroyed.push_back(enemie);
						}
						else if (enemie->GetFormStateId() == DELETE_BY_BLOCK) {
							enemie->SetVisibility(false);
							toBeDestroyedByBlock.push_back(enemie);
						}
						else {
							int speed = 0;
							if (enemie_type == "goomba")
								speed = ENEMIES_MOVE_SPEED;
							else if (enemie->GetStateId() == WALKING_STATE) {
								speed = ENEMIES_MOVE_SPEED;
								if (enemie->GetFormStateId() == SMASHED)
									speed = SHELL_SPEED;
							}
							enemie->Move(enemie->lastMovedRight ? speed : -speed, 0);
						}
					}

				npcs_types = { "piranha_plant", "coin"};
				for (std::string npc_type : npcs_types)
					for (auto npc : SpriteManager::GetSingleton().GetTypeList(npc_type)) {
						if (npc->GetFormStateId() == DELETE) {
							npc->SetVisibility(false);
							toBeDestroyed.push_back(npc);
						}
					}

				for (auto powerup : SpriteManager::GetSingleton().GetTypeList("powerup")) {
					if (powerup->GetFormStateId() == DELETE || powerup->GetBox().y > (action_layer->GetViewWindow().y + action_layer->GetViewWindow().h)) {
						powerup->SetVisibility(false);
						toBeDestroyed.push_back(powerup);
					}
					else
						powerup->Move(powerup->lastMovedRight ? POWERUPS_MOVE_SPEED : -POWERUPS_MOVE_SPEED, 0);
				}
				for (auto sprite : toBeDestroyed) {
					if (sprite->GetTypeId() == "goomba" || sprite->GetTypeId() == "green_koopa_troopa" || sprite->GetTypeId() == "red_koopa_troopa")
						game.addPoints(100);
					else if (sprite->GetTypeId() == "piranha_plant")
						game.addPoints(200);
					else if (sprite->GetTypeId() == "powerup")
						game.addPoints(1000);
					SpriteManager::GetSingleton().Remove(sprite);
					CollisionChecker::GetSingleton().CancelAll(sprite);
				}
				for (auto sprite : toBeDestroyedByBlock) {
					if (sprite->GetTypeId() == "goomba" || sprite->GetTypeId() == "green_koopa_troopa" || sprite->GetTypeId() == "red_koopa_troopa")
						game.addPoints(100);
					else if (sprite->GetTypeId() == "piranha_plant")
						game.addPoints(200);
					sprite->GetAnimator()->Stop();
					sprite->GetAnimator()->deleteCurrAnimation();
					sprite->GetAnimator()->Destroy();
					AnimatorManager::GetSingleton().Cancel(sprite->GetAnimator());
					SpriteManager::GetSingleton().Remove(sprite);
					CollisionChecker::GetSingleton().CancelAll(sprite);
				}
				toBeDestroyed.clear();
				toBeDestroyedByBlock.clear();
			}

			if (!al_is_event_queue_empty(blockQueue)) {
				al_wait_for_event(blockQueue, &event);
				for (auto block : SpriteManager::GetSingleton().GetTypeList("block")) {
					if (block->GetFormStateId() == MOVED_BLOCK) {
						block->Move(0, 4);
						block->SetFormStateId(EMPTY_BLOCK);
						block->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm("blocks.empty_block"));
						al_stop_timer(blockTimer);
					}
				}
				for (auto brick : SpriteManager::GetSingleton().GetTypeList("brick")) {
					if (brick->GetFormStateId() == MOVED_BLOCK) {
						brick->Move(0, 4);
						brick->SetFormStateId(BRICK);
						al_stop_timer(blockTimer);
					}
					else if (brick->GetFormStateId() == SMASHED) {
						toBeDestroyed.push_back(brick);
						action_layer->UnsolidTileGridBlocks(DIV_TILE_WIDTH(brick->GetBox().x + action_layer->GetViewWindow().x), DIV_TILE_HEIGHT(brick->GetBox().y + action_layer->GetViewWindow().y) + 1);
					}
				}
				for (auto sprite : toBeDestroyed) {
					SpriteManager::GetSingleton().Remove(sprite);
					CollisionChecker::GetSingleton().CancelAll(sprite);
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

	blockQueue = al_create_event_queue();
	blockTimer = al_create_timer(1.0/6);
	al_register_event_source(blockQueue, al_get_timer_event_source(blockTimer));

	font = al_load_font(".\\Engine\\Media\\game_font.ttf", 20, NULL);
	paused_font = al_load_font(".\\Engine\\Media\\game_font.ttf", 40, NULL);
	tittle_font = al_load_font(".\\Engine\\Media\\game_font.ttf", 30, NULL);
	tittle_font_smaller = al_load_font(".\\Engine\\Media\\game_font.ttf", 25, NULL);

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
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".stand_right"));
		else
			mario->SetCurrFilm(AnimationFilmHolder::GetInstance().GetFilm(mario->Get_Str_StateId() + ".stand_left"));
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
		 + "Mario_big.walk_right:" + string(al_get_config_value(config, "Mario_big", "walk_right")) + '$'
		 + "Mario_big.walk_left:" + string(al_get_config_value(config, "Mario_big", "walk_left")) + '$'
		 + "Mario_big.stand_right:" + string(al_get_config_value(config, "Mario_big", "stand_right")) + '$'
		 + "Mario_big.stand_left:" + string(al_get_config_value(config, "Mario_big", "stand_left")) + '$'
		 + "Mario_big.jump_right:" + string(al_get_config_value(config, "Mario_big", "jump_right")) + '$'
		 + "Mario_big.jump_left:" + string(al_get_config_value(config, "Mario_big", "jump_left")) + '$'
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
		+ "blocks.empty_block:" + string(al_get_config_value(config, "blocks", "empty_block")) + '$'
		+ "blocks.coin:" + string(al_get_config_value(config, "blocks", "coin")) + '$'
		;
}

string loadAllPowerups(const ALLEGRO_CONFIG* config) {

	return "powerups.super:" + string(al_get_config_value(config, "powerups", "super")) + '$'
		+ "powerups.1up:" + string(al_get_config_value(config, "powerups", "1up")) + '$'
		+ "powerups.star:" + string(al_get_config_value(config, "powerups", "star")) + '$'
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
	menu_circular_background = new CircularBackground(bg_tiles, al_get_config_value(config, "paths", "menu_backround_path"));

	loadSolidTiles(config, action_layer);
	action_layer->ComputeTileGridBlocks1();

	AnimationFilmHolder::GetInstance().LoadAll(loadAllCharacters(config), al_get_config_value(config, "paths", "characters_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllPipes(config), al_get_config_value(config, "paths", "tiles_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllEnemies(config), al_get_config_value(config, "paths", "enemies_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllBlocks(config), al_get_config_value(config, "paths", "npcs_path"));
	AnimationFilmHolder::GetInstance().LoadAll(loadAllPowerups(config), al_get_config_value(config, "paths", "npcs_path"));

	liveIcon = SubBitmapCreate(BitmapLoad(al_get_config_value(config, "paths", "characters_path")), { 127, 60, 16, 16 });
	coinIcon = SubBitmapCreate(BitmapLoad(al_get_config_value(config, "paths", "npcs_path")), {0, 16, 16, 16});

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
	mario->Set_Str_StateId("Mario_small");

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

	//create super mushroom
	locations = splitString(al_get_config_value(config, "powerups_positions", "super"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_super_mushroom(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}

	//create 1up mushroom
	locations = splitString(al_get_config_value(config, "powerups_positions", "1up"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_1UP_mushroom(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()), &game);
	}

	//create star
	locations = splitString(al_get_config_value(config, "powerups_positions", "star"), ",");
	for (auto location : locations) {
		coordinates = splitString(location, " ");
		create_starman(atoi(coordinates[0].c_str()), atoi(coordinates[1].c_str()));
	}

	for (unsigned int i = 0; i < action_layer->GetMapWidth(); i++) {
		for (unsigned int j = 0; j < action_layer->GetMapHeight(); j++) {
			// replace each brick index with sprite
			if (action_layer->GetTile(j, i) == 0) {
				create_block_sprite(MUL_TILE_WIDTH(i), MUL_TILE_HEIGHT(j), &game);
			}

			// replace each block index with sprite
			if (action_layer->GetTile(j, i) == 4) {
				create_brick_sprite(MUL_TILE_WIDTH(i), MUL_TILE_HEIGHT(j));
			}

			// replace each coin index with sprite
			if (action_layer->GetTile(j, i) == 26) {
				create_coin_sprite(MUL_TILE_WIDTH(i), MUL_TILE_HEIGHT(j), &game);
			}
		}
	}
}

void mainMenu() {
	al_flush_event_queue(queue);
	int total_scroll = 0;
	while(true){
		menu_circular_background->Display(al_get_backbuffer(display), displayArea.x, displayArea.y);

		al_draw_text(tittle_font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 30, ALLEGRO_ALIGN_CENTER, "CSD4022 - ANTONIS PARAGIOUDAKIS");
		al_draw_text(tittle_font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 70, ALLEGRO_ALIGN_CENTER, "CSD4101 - MIXALIS RAPTAKIS");
		al_draw_text(tittle_font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 110, ALLEGRO_ALIGN_CENTER, "CSD4017 - GEORGOS PANTELAKIS");
		al_draw_text(tittle_font_smaller, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 220, ALLEGRO_ALIGN_CENTER, "University of Crete");
		al_draw_text(tittle_font_smaller, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 250, ALLEGRO_ALIGN_CENTER, "Department of Computer Science");
		al_draw_text(tittle_font_smaller, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 280, ALLEGRO_ALIGN_CENTER, "CS-454. Development of Intelligent Interfaces and Games");
		al_draw_text(tittle_font_smaller, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 310, ALLEGRO_ALIGN_CENTER, "Term Project, Fall Semester 2020");
		al_draw_text(font, al_map_rgb(0, 0, 0), action_layer->GetViewWindow().w / 2, 420, ALLEGRO_ALIGN_CENTER, "Press ENTER to continue...");


		al_flip_display();

		if (!al_is_event_queue_empty(queue)) {
			al_wait_for_event(queue, &event);
			if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				delete menu_circular_background;
				break;
			}
			else if (event.type == ALLEGRO_EVENT_TIMER) {
				menu_circular_background->Scroll(1);
				total_scroll++;
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				closeWindowClicked = true;
				break;
			}
		}
	}
}

void app::App::Run(void) {
	mainMenu();
	al_flush_event_queue(fallingQueue);
	al_flush_event_queue(aiQueue);
	game.MainLoop();
}

void app::MainApp::Clear(void) {
	al_destroy_font(font);
	al_destroy_font(paused_font);
	al_destroy_font(tittle_font);
	al_destroy_font(tittle_font_smaller);
	al_destroy_display(display);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(action_layer->GetBitmap());
	al_destroy_bitmap(underground_layer->GetBitmap());
	//TODO destroy grid, tiles, background
	delete action_layer;
	delete underground_layer;
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

int app::Game::getLives(void) {
	return lives;
}

void app::Game::addLife(void) {
	lives++;
}

void app::Game::loseLife(void) {
	lives--;
}

bool app::Game::isDead(void) {
	return lives == 0;
}

int app::Game::getCoins(void) {
	return coins;
}

void app::Game::addCoin(void) {
	coins += 1;

	if (coins == 100) {
		resetCoins();
		addLife();
	}
}

void app::Game::resetCoins(void) {
	coins = 0;
}

int app::Game::getPoints(void) {
	return points;
}

void app::Game::addPoints(int extra_points) {
	points += extra_points;
}