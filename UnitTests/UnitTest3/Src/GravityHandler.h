#pragma once
#include "app.h"

class GravityHandler {
public:
	using OnSolidGroundPred = std::function<bool(const Rect&)>;
	using OnStartFalling = std::function<void(void)>;
	using OnStopFalling = std::function<void(void)>;

protected:
	bool gravityAddicted = false;
	bool isFalling = false;
	OnSolidGroundPred onSolidGround;
	OnStartFalling onStartFalling;
	OnStopFalling onStopFalling;

public:
	template<typename T>
	void SetOnStartFalling(const T& f);
	template<typename T>
	void SetOnStopFalling(const T& f);
	template<typename T>
	void SetOnSolidGround(const T& f);
	void Reset(void);
	void Check(const Rect& r);
};