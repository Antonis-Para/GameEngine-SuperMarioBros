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

bool app::Clipper::Clip(const Rect& r, const Rect& dpyArea, Point* dpyPos, Rect* clippedBox) const {
	Rect visibleArea;
	if (!clip_rect(r, view(), &visibleArea)) {
		clippedBox->w = clippedBox->h = 0;
		return false;
	} else {
		clippedBox->x = r.x - visibleArea.x;
		clippedBox->y = r.y - visibleArea.y;
		clippedBox->w = visibleArea.w;
		clippedBox->h = visibleArea.h;
		dpyPos->x = dpyArea.x + (visibleArea.x - view().x);
		dpyPos->y = dpyArea.y + (visibleArea.y - view().y);
		return true;
	}
}

// Sprite
app::Sprite::Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId) :
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
	if (directMotion) // apply unconditionally offsets!
		x += dx, y += dy;
	else {
		quantizer.Move(GetBox(), &dx, &dy);
		gravity.Check(GetBox());
	}
	//quantizer.Move(GetBox(), &dx, &dy);
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
	//boundingArea = area.Clone();
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

void app::Sprite::Display(Bitmap dest, const Rect& dpyArea, const Clipper& clipper) const {
	Rect clippedBox;
	Point  dpyPos;
	if (clipper.Clip(GetBox(), dpyArea, &dpyPos, &clippedBox)) {
		Rect clippedFrame { frameBox.x + clippedBox.x, frameBox.y + clippedBox.y, clippedBox.w, clippedBox.h };
		BitmapBlit(currFilm->GetBitmap(), clippedFrame, dest, dpyPos); // MaskedBlit
	}
}

GravityHandler& app::Sprite::GetGravityHandler(void) {
	return gravity;
}

void app::Sprite::SetHasDirectMotion(bool v) {
	directMotion = true;
}

bool app::Sprite::GetHasDirectMotion(void) const {
	return directMotion;
}

// CollisionChecker
app::CollisionChecker app::CollisionChecker::singleton;

template<typename T>
void app::CollisionChecker::Register(Sprite* s1, Sprite* s2, const T& f) {
	entries.push_back(std::make_tuple(s1, s2, f));
}

void app::CollisionChecker::Cancel(Sprite* s1, Sprite* s2) {
	auto i = std::find_if(entries.begin(), entries.end(), [s1, s2](const Entry& e) {
		return std::get<0>(e) == s1 && std::get<1>(e) == s2 || std::get<0>(e) == s2 && std::get<1>(e) == s1;
	});
	entries.erase(i);
}

void app::CollisionChecker::Check(void) const {
	for (auto& e : entries)
		if (std::get<0>(e)->CollisionCheck(std::get<1>(e)))
			std::get<2>(e)(std::get<0>(e), std::get<1>(e));
}

auto app::CollisionChecker::GetSingleton(void) -> CollisionChecker& {
	return singleton;
}

auto app::CollisionChecker::GetSingletonConst(void) -> const CollisionChecker& {
	return singleton;
}

// SpriteManager
app::SpriteManager app::SpriteManager::singleton;

void app::SpriteManager::Add(Sprite* s) {
	dpyList.push_back(s);
	dpyList.sort([](Sprite* s1, Sprite* s2) -> bool { return s1->GetFrame() < s2->GetFrame(); });
}

void app::SpriteManager::Remove(Sprite* s) {
	dpyList.remove(s);
}

auto app::SpriteManager::GetDisplayList(void) -> const SpriteList& {
	return dpyList;
}

auto app::SpriteManager::GetTypeList(const std::string& typeId) -> const SpriteList& {
	return types[typeId];
}

auto app::SpriteManager::GetSingleton(void) -> SpriteManager& {
	return singleton;
}

auto app::SpriteManager::GetSingletonConst(void) -> const SpriteManager& {
	return singleton;
}

// General functions
const app::Clipper app::MakeTileLayerClipper(TileLayer* layer) {
	return Clipper().SetView([layer](void) { return layer->GetViewWindow(); });
}

const app::Sprite::Mover app::MakeSpriteGridLayerMover(GridLayer* gridLayer, Sprite* sprite) {
	return [gridLayer, sprite](const Rect & rect, int* dx, int* dy) {
		gridLayer->FilterGridMotion(sprite->GetBox(), dx, dy);
	};
}

void app::PrepareSpriteGravityHandler(GridLayer* gridLayer, Sprite* sprite) {
	sprite->GetGravityHandler().SetOnSolidGround([gridLayer](const Rect& r) {
		return gridLayer->IsOnSolidGround(r);
	});
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