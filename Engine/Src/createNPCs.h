#include "Sprite.h"
#include <cstdlib>
#include <ctime>

namespace app {

	//create enemies
	void create_enemy_goomba(int x, int y);
	void create_enemy_green_koopa_troopa(int x, int y);
	void create_enemy_red_koopa_troopa(int x, int y);
	void create_enemy_piranha_plant(int x, int y);

	//create powerups
	void create_super_mushroom(int x, int y);
	void create_1UP_mushroom(int x, int y, Game* game);
	void create_starman(int x, int y);

	//create blocks
	void create_brick_sprite(int x, int y);
	void create_block_sprite(int, int, Game*);
	void create_coin_sprite(int x, int y, Game* game);
	Sprite* LoadPipeCollision(Sprite* mario, string pipes);
}