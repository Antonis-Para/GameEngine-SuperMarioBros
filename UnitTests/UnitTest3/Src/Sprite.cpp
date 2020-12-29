#include "Sprite.h"

// MotionQuantizer
app::MotionQuantizer& app::MotionQuantizer::SetRange(int h, int v) {
	horizMax = h, vertMax = v;
	used = true;
	return *this;
}

template<typename Tfunc>
app::MotionQuantizer& app::MotionQuantizer::SetMover(const Tfunc& f) {
	mover = f;
	return *this;
}

void app::MotionQuantizer::Move(const Rect& r, int* dx, int* dy) {
	if (!used)
		mover(r, dx, dy);
	else
		do {
			auto dxFinal = std::min(number_sign(*dx) * horizMax, *dx);
			auto dyFinal = std::min(number_sign(*dy) * vertMax, *dy);
			mover(r, &dxFinal, &dyFinal);
			if (!dxFinal) // X motion denied
				*dx = 0;
			else
				*dx -= dxFinal;
			if(!dyFinal)// Y motion denied
				*dy = 0;
			else
				*dy -= dyFinal;
		} while(*dx|| *dy);
}

// Clipper
template<typename Tfunc>
app::Clipper& app::Clipper::SetView(const Tfunc& f) {
	view = f;
	return *this;
}

// Sprite
app::Sprite::Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId = "") :
	x(_x), y(_y), currFilm(film), typeId(_typeId) {
	frameNo = currFilm->GetTotalFrames();
	SetFrame(0);
}

template<typename Tfunc>
void app::Sprite::SetMover(const Tfunc& f) {
	quantizer.SetMover(mover = f);
}

const Rect app::Sprite::GetBox(void) const {
	return { x, y, frameBox.w, frameBox.h };
}

void app::Sprite::Move(int dx, int dy) {
	quantizer.Move(GetBox(), &dx, &dy);
}

void app::Sprite::SetPos(int _x, int _y) {
	x = _x; y = _y;
}

void app::Sprite::SetZorder(unsigned z) {
	zorder = z;
}

unsigned app::Sprite::GetZorder(void) {
	return zorder;
}

void app::Sprite::SetFrame(unsigned char i) {
	if (i != frameNo) {
		assert(i < currFilm->GetTotalFrames());
		frameBox = currFilm->GetFrameBox(frameNo = i);
	}
}

unsigned char app::Sprite::GetFrame(void) const {
	return frameNo;
}

void app::Sprite::SetBoundingArea(const BoundingArea& area) {
	assert(!boundingArea);
	boundingArea = area.Clone();
}

void app::Sprite::SetBoundingArea(BoundingArea* area) {
	assert(!boundingArea);
	boundingArea = area;
}

auto app::Sprite::GetBoundingArea(void) const -> const BoundingArea* {
	return boundingArea;
}

auto app::Sprite::GetTypeId(void) -> const std::string& {
	return typeId;
}

void app::Sprite::SetVisibility(bool v) {
	isVisible = v;
}

bool app::Sprite::IsVisible(void) const {
	return isVisible;
}

template<typename Tnum>
int app::number_sign(Tnum x) {
	return x > 0 ? 1 : x < 0 ? -1 : 0;
}

template<class T>
bool app::clip_rect(T x, T y, T w, T h, T wx, T wy, T ww, T wh, T* cx, T* cy, T* cw, T* ch) {
	*cw = T(std::min(wx + ww, x + w)) - (*cx = T(std::max(wx, x)));
	*ch = T(std::min(wy + wh, y + h)) - (*cy = T(std::max(wy, y)));
	return *cw > 0 && *ch > 0;
}

bool app::clip_rect(const Rect& r, const Rect& area, Rect* result) {
	return clip_rect(r.x, r.y, r.w, r.h, area.x, area.y, area.w, area.h, &result->x, &result->y, &result->w, &result->h);
}