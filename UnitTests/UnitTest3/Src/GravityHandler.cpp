#include "GravityHandler.h"

void GravityHandler::Reset(void) {
	isFalling = false;
}

void GravityHandler::Check(const Rect& r) {
	if (gravityAddicted) {
		if (onSolidGround(r)) {
			if (isFalling) {
				isFalling = false;
				onStopFalling();
			}
		} else if(!isFalling) {
			isFalling = true;
			onStartFalling();
		}
	}
}

