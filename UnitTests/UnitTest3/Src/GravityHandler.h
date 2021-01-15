#pragma once
#include <functional>
#include "Shapes.h"

class GravityHandler {
public:
	using OnSolidGround = std::function<bool(const Rect& r)>;
	using OnStartFalling = std::function<void(void)>;
	using OnStopFalling = std::function<void(void)>;

protected:
	bool gravityAddicted = false;
	bool is_Falling = false;
	OnSolidGround onSolidGround;
	OnStartFalling onStartFalling;
	OnStopFalling onStopFalling;

public:
	template<typename T>
	void SetOnStartFalling(const T& f) { onStartFalling = f; }
	template<typename T>
	void SetOnStopFalling(const T& f) { onStopFalling = f; }
	template<typename T>
	void SetOnSolidGround(const T& f) { onSolidGround = f; }
	void Reset(void);
	void Check(const Rect& r);
	bool isFalling();
};