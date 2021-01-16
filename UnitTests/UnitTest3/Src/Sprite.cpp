#include "Sprite.h"

// MotionQuantizer
MotionQuantizer& MotionQuantizer::SetRange(int h, int v) {
	horizMax = h, vertMax = v;
	used = true;
	return *this;
}

//template<typename Tfunc>
//MotionQuantizer& MotionQuantizer::SetMover(const Tfunc& f) {
//	mover = f;
//	return *this;
//}

void MotionQuantizer::Move(const Rect& r, int* dx, int* dy) {
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
Clipper& Clipper::SetView(const Tfunc& f) {
	view = f;
	return *this;
}

bool Clipper::Clip(const Rect& r, const Rect& dpyArea, Point* dpyPos, Rect* clippedBox) const {
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
Sprite::Sprite(int _x, int _y, const AnimationFilm* film, const std::string& _typeId) :
	x(_x), y(_y), currFilm(film), typeId(_typeId) {
	frameNo = currFilm->GetTotalFrames();
	SetFrame(0);
}

const Rect Sprite::GetBox(void) const {
	return { x, y, frameBox.w, frameBox.h };
}

Sprite& Sprite::Move(int dx, int dy) {
	if (directMotion) // apply unconditionally offsets!
		x += dx, y += dy;
	else {
		quantizer.Move(GetBox(), &dx, &dy);
		gravity.Check(GetBox());
	}
	return *this;
}

void Sprite::SetPos(int _x, int _y) {
	x = _x; y = _y;
}

void Sprite::SetZorder(unsigned z) {
	zorder = z;
}

unsigned Sprite::GetZorder(void) {
	return zorder;
}

const AnimationFilm* Sprite::GetCurrFilm() {
	return currFilm;
}

void Sprite::SetCurrFilm(const AnimationFilm* newFilm) {
	currFilm = newFilm;
}

void Sprite::NextFrame() {
	frameNo++;
	if (currFilm->GetTotalFrames() <= frameNo)
		frameNo = 0; //start over
}

void Sprite::SetFrame(unsigned char i) {
	if (i != frameNo) {
		assert(i < currFilm->GetTotalFrames());
		frameBox = currFilm->GetFrameBox(frameNo = i);
	}
}

unsigned char Sprite::GetFrame(void) const {
	return frameNo;
}

void Sprite::SetBoundingArea(const BoundingArea& area) {
	assert(!boundingArea);
	//boundingArea = area.Clone();
}

void Sprite::SetBoundingArea(BoundingArea* area) {
	assert(!boundingArea);
	boundingArea = area;
}

auto Sprite::GetBoundingArea(void) const -> const BoundingArea* {
	return boundingArea;
}

auto Sprite::GetTypeId(void) -> const std::string& {
	return typeId;
}

spritestate_t Sprite::GetStateId(void){
	return stateId;
}

void Sprite::SetStateId(spritestate_t id){
	stateId = id;
}

void Sprite::SetVisibility(bool v) {
	isVisible = v;
}

bool Sprite::IsVisible(void) const {
	return isVisible;
}

void Sprite::Display(Bitmap dest, const Rect& dpyArea, const Clipper& clipper) const {
	Rect clippedBox;
	Point  dpyPos;
	if (clipper.Clip(GetBox(), dpyArea, &dpyPos, &clippedBox)) {
		Rect clippedFrame { frameBox.x + clippedBox.x, frameBox.y + clippedBox.y, clippedBox.w, clippedBox.h };
		BitmapBlit(currFilm->GetBitmap(), clippedFrame, dest, dpyPos); // MaskedBlit
	}
}

const Sprite::Mover MakeSpriteGridLayerMover(GridLayer* gridLayer, Sprite* sprite) {
	return [gridLayer, sprite](const Rect& rect, int* dx, int* dy) {
		gridLayer->FilterGridMotion(sprite->GetBox(), dx, dy);
	};
}

GravityHandler& Sprite::GetGravityHandler(void) {
	return gravity;
}

void Sprite::SetHasDirectMotion(bool v) {
	directMotion = true;
}

bool Sprite::GetHasDirectMotion(void) const {
	return directMotion;
}

// CollisionChecker
/*CollisionChecker CollisionChecker::singleton;

template<typename T>
void CollisionChecker::Register(Sprite* s1, Sprite* s2, const T& f) {
	entries.push_back(std::make_tuple(s1, s2, f));
}

void CollisionChecker::Cancel(Sprite* s1, Sprite* s2) {
	auto i = std::find_if(entries.begin(), entries.end(), [s1, s2](const Entry& e) {
		return std::get<0>(e) == s1 && std::get<1>(e) == s2 || std::get<0>(e) == s2 && std::get<1>(e) == s1;
	});
	entries.erase(i);
}

void CollisionChecker::Check(void) const {
	for (auto& e : entries)
		if (std::get<0>(e)->CollisionCheck(std::get<1>(e)))
			std::get<2>(e)(std::get<0>(e), std::get<1>(e));
}

auto CollisionChecker::GetSingleton(void) -> CollisionChecker& {
	return singleton;
}

auto CollisionChecker::GetSingletonConst(void) -> const CollisionChecker& {
	return singleton;
}
*/

// SpriteManager
SpriteManager SpriteManager::singleton;

void SpriteManager::Add(Sprite* s) {
	dpyList.push_back(s);
	dpyList.sort([](Sprite* s1, Sprite* s2) -> bool { return s1->GetFrame() < s2->GetFrame(); });
}

void SpriteManager::Remove(Sprite* s) {
	dpyList.remove(s);
}

auto SpriteManager::GetDisplayList(void) -> const SpriteList& {
	return dpyList;
}

auto SpriteManager::GetTypeList(const std::string& typeId) -> const SpriteList& {
	return types[typeId];
}

auto SpriteManager::GetSingleton(void) -> SpriteManager& {
	return singleton;
}

auto SpriteManager::GetSingletonConst(void) -> const SpriteManager& {
	return singleton;
}

// General functions
const Clipper MakeTileLayerClipper(TileLayer* layer) {
	return Clipper().SetView([layer](void) { return layer->GetViewWindow(); });
}

extern class TileLayer* action_layer;
void PrepareSpriteGravityHandler(GridLayer* gridLayer, Sprite* sprite) {
	sprite->GetGravityHandler().SetOnSolidGround([gridLayer](const Rect& r) {
		Rect posOnGrid{
			r.x + action_layer->GetViewWindow().x,
			r.y + action_layer->GetViewWindow().y,
			r.w,
			r.h,
		};
		return gridLayer->IsOnSolidGround(posOnGrid);
	});
}

bool clip_rect(const Rect& r, const Rect& area, Rect* result) {
	return clip_rect(r.x, r.y, r.w, r.h, area.x, area.y, area.w, area.h, &result->x, &result->y, &result->w, &result->h);
}