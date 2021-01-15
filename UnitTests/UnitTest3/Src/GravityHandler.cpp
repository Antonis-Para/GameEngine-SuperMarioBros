#include "GravityHandler.h"
#include <sstream>
#include <stdio.h>
#include <iostream>

void GravityHandler::Reset(void) {
	is_Falling = false;
}

void GravityHandler::Check(const Rect& r) {
	if (gravityAddicted) {
		if (onSolidGround(r)) {
			if (is_Falling) {
				is_Falling = false;
				//onStopFalling();
			}
		} else if(!is_Falling) {
			is_Falling = true;
			//onStartFalling();
		}
	}
}

bool GravityHandler::isFalling() {
	return is_Falling;

}

void GravityHandler::setGravityAddicted(bool val) {
	gravityAddicted = val;
}