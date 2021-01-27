#include "Sprite.h"

namespace app {

	//create enemies
	void create_enemy_goomba(int x, int y);
	void create_enemy_green_koopa_troopa(int x, int y);
	void create_enemy_red_koopa_troopa(int x, int y);
	void create_enemy_piranha_plant(int x, int y);

	//create blocks
	void create_brick_sprite(int x, int y);
	void create_block_sprite(int x, int y);
	Sprite* LoadPipeCollision(Sprite* mario, string pipes);
}