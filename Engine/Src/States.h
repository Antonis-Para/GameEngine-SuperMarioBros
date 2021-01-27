#pragma once

enum spritestate_t {
	IDLE_STATE = 0, WALKING_STATE = 1, RUNNING_STATE = 2
};

enum spriteFormState_t {
	SMALL_MARIO = 0,
	SUPER_MARIO = 1,
	INVINCIBLE_MARIO = 2,
	ENEMY = 3,
	SMASHED = 4,
	PIPE = 5,
	DELETE = 6,
	BRICK = 7,
	BLOCK = 8,
	EMPTY_BLOCK = 9,
	MOVED_BLOCK = 10,
	DELETE_BY_BLOCK = 11,
	COIN = 12
};
