#include "GravityHandler.h"

//template<typename T>
//void GravityHandler::SetOnStartFalling(const T& f) {
//	onStartFalling = f;
//}
//
//template<typename T>
//void GravityHandler::SetOnStopFalling(const T& f) {
//	onStopFalling = f;
//}
//
//template<typename T>
//void GravityHandler::SetOnSolidGround(const T& f) {
//	onSolidGround = f;
//}

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

