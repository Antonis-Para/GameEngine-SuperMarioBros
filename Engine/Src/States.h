#pragma once

enum spritestate_t {
	IDLE_STATE = 0, WALKING_STATE = 1, RUNNING_STATE = 2
};

enum spriteFormState_t {
	SMALL_MARIO = 0, 
	SUPER_MARIO = 1, 
	ENEMY = 2, 
	SMASHED = 3, 
	PIPE = 4, 
	DELETE = 5, 
	BRICK = 6, 
	BLOCK = 7, 
	EMPTY_BLOCK = 8,
	MOVED_BLOCK = 9
};
