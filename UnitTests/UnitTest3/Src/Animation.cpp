#include "Animation.h"

// AnimationFilm
app::AnimationFilm::AnimationFilm(const std::string& _id) : id(_id) {}
app::AnimationFilm::AnimationFilm(Bitmap, const std::vector<Rect>&, const std::string&) {}

unsigned char app::AnimationFilm::GetTotalFrames(void) const {
	return boxes.size();
}

Bitmap app::AnimationFilm::GetBitmap(void) const {
	return bitmap;
}

auto app::AnimationFilm::GetId(void) const -> const std::string& {
	return id;
}

const Rect& app::AnimationFilm::GetFrameBox(unsigned char frameNo) const {
	assert(boxes.size() > frameNo);
	return boxes[frameNo];
}

void app::AnimationFilm::DisplayFrame(Bitmap dest, const Point& at, unsigned char frameNo) const {
	BitmapBlit(bitmap, GetFrameBox(frameNo), dest, at); // MaskedBlit
}

void app::AnimationFilm::SetBitmap(Bitmap b) {
	assert(!bitmap); bitmap = b;
}

void app::AnimationFilm::Append(const Rect& r) {
	boxes.push_back(r);
}

// AnimationFilmHolder
auto app::AnimationFilmHolder::Get(void) -> const AnimationFilmHolder& {
	return holder;
}
// TODO(4u): set a parsing functor implemented externally to the class
int app::AnimationFilmHolder::ParseEntry( // -1=error, 0=ended gracefully, else #chars read
	int startPos, const std::string& text, std::string& id, std::string& path, std::vector<Rect>& rects) {

}
void app::AnimationFilmHolder::LoadAll(const std::string& text) {
	int pos = 0;
	while (true) {
		std::string id, path;
		std::vector<Rect> rects;
		auto i = ParseEntry(pos, text, id, path, rects);
		assert(i >= 0);
		if (!i) return;
		pos += i;
		assert(!GetFilm(id));
		films[id] = new AnimationFilm(bitmaps.Load(path), rects, id);
	}
}

void app::AnimationFilmHolder::CleanUp(void) {
	for (auto& i : films)
		delete (i.second);
	films.clear();
}

auto app::AnimationFilmHolder::GetFilm(const std::string& id) -> const AnimationFilm* const {
	auto i = films.find(id);
	return i != films.end() ? i->second : nullptr;
}

// BitmapLoader
app::BitmapLoader::BitmapLoader(void) {}
app::BitmapLoader::~BitmapLoader() {
	CleanUp();
}

Bitmap app::BitmapLoader::GetBitmap(const std::string& path) const {
	auto i = bitmaps.find(path);
	return i != bitmaps.end() ? i->second : nullptr;
}

Bitmap app::BitmapLoader::Load(const std::string& path) {
	auto b = GetBitmap(path);
	if (!b) {
		bitmaps[path] = b = BitmapLoad(path);
		assert(b);
	}
	return b;
}

// prefer to massively clear bitmaps at the end than
// to destroy individual bitmaps during gameplay
void  app::BitmapLoader::CleanUp(void) {
	for (auto& i : bitmaps)
		BitmapDestroy(i.second);
	bitmaps.clear();
}

// Animation
app::Animation::Animation(const std::string& _id) : id(_id) {}
app::Animation::~Animation() {}

const std::string& app::Animation::GetId(void) {
	return id;
}

void app::Animation::SetId(const std::string& _id) {
	id = _id;
}

// MovingAnimation
app::MovingAnimation::MovingAnimation(const std::string& _id, unsigned _reps, int _dx, int _dy, unsigned _delay) :
	Animation(_id), reps(_reps), dx(_dx), dy(_dy), delay(_delay) {}

int app::MovingAnimation::GetDx(void) const {
	return dx;
}

app::MovingAnimation::Me& app::MovingAnimation::SetDx(int v) {
	dx = v;
	return *this;
}

int app::MovingAnimation::GetDy(void) const {
	return dy;
}

app::MovingAnimation::Me& app::MovingAnimation::SetDy(int v) {
	dy = v;
	return *this;
}

unsigned app::MovingAnimation::GetDelay(void) const {
	return delay;
}

app::MovingAnimation::Me& app::MovingAnimation::SetDelay(unsigned v) {
	delay = v;
	return *this;
}

unsigned app::MovingAnimation::GetReps(void) const {
	return reps;
}

app::MovingAnimation::Me& app::MovingAnimation::SetReps(unsigned n) {
	reps = n;
	return *this;
}

bool app::MovingAnimation::IsForever(void) const {
	return !reps;
}

app::MovingAnimation::Me& app::MovingAnimation::SetForever(void) {
	reps = 0;
	return *this;
}

app::Animation* app::MovingAnimation::Clone(void) const {
	return new MovingAnimation(id, reps, dx, dy, delay);
}

// FrameRangeAnimation
app::FrameRangeAnimation::FrameRangeAnimation(const std::string& _id, unsigned s, unsigned e, unsigned r, int dx, int dy, int d):
	start(s), end(e), MovingAnimation(id, r, dx, dy, d) {}

unsigned app::FrameRangeAnimation::GetStartFrame(void) const {
	return start;
}

app::FrameRangeAnimation::Me& app::FrameRangeAnimation::SetStartFrame(unsigned v) {
	start = v;
	return *this;
}

unsigned app::FrameRangeAnimation::GetEndFrame(void) const {
	return end;
}

app::FrameRangeAnimation::Me& app::FrameRangeAnimation::SetEndFrame(unsigned v) {
	end = v;
	return *this;
}

app::Animation* app::FrameRangeAnimation::Clone(void) const {
	return new FrameRangeAnimation(id, start, end, GetReps(), GetDx(), GetDy(), GetDelay());
}

// FrameListAnimation
app::FrameListAnimation::FrameListAnimation(const std::string& _id, const Frames& _frames, unsigned r, int dx, int dy, unsigned d, bool c) :
	frames(_frames), MovingAnimation(id, r, dx, dy, d) {}

const app::FrameListAnimation::Frames& app::FrameListAnimation::GetFrames(void) const {
	return frames;
}

void app::FrameListAnimation::SetFrames(const Frames& f) {
	frames = f;
}

app::Animation* app::FrameListAnimation::Clone(void) const {
	return new FrameListAnimation(id, frames, GetReps(), GetDx(), GetDy(), GetDelay(), true);
}

// MovingPathAnimation
app::MovingPathAnimation::MovingPathAnimation(const std::string& _id, const Path& _path) :
	path(_path), Animation(id) {}

const app::MovingPathAnimation::Path& app::MovingPathAnimation::GetPath(void) const {
	return path;
}

void app::MovingPathAnimation::SetPath(const Path& p) {
	path = p;
}

app::Animation* app::MovingPathAnimation::Clone(void) const {
	return new MovingPathAnimation(id, path);
}

// FlashAnimation
app::FlashAnimation::FlashAnimation(const std::string& _id, unsigned n, unsigned show, unsigned hide) :
	Animation(id), repetitions(n), hideDelay(hide), showDelay(show) {}

app::FlashAnimation::Me& app::FlashAnimation::SetRepetitions(unsigned n) {
	repetitions = n;
	return *this;
}

unsigned app::FlashAnimation::GetRepetitions(void) const {
	return repetitions;
}

app::FlashAnimation::Me& app::FlashAnimation::SetHideDeay(unsigned d) {
	hideDelay = d;
	return *this;
}

unsigned app::FlashAnimation::GetHideDeay(void) const {
	return hideDelay;
}

app::FlashAnimation::Me& app::FlashAnimation::SetShowDeay(unsigned d) {
	showDelay = d;
	return *this;
}

unsigned app::FlashAnimation::GetShowDeay(void) const {
	return showDelay;
}

app::Animation* app::FlashAnimation::Clone(void) const {
	return new FlashAnimation(id, repetitions, hideDelay, showDelay);
}

// ScrollAnimation
app::ScrollAnimation::ScrollAnimation(const std::string& _id, const Scroll& _scroll) :
	Animation(_id), scroll(_scroll) {}

const app::ScrollAnimation::Scroll& app::ScrollAnimation::GetScroll(void) const {
	return scroll;
}

void app::ScrollAnimation::SetScroll(const Scroll& p) {
	scroll = p;
}

app::Animation* app::ScrollAnimation::Clone(void) const {
	return new ScrollAnimation(id, scroll);
}

void app::Animate(AnimationFilm& film, const Point at) {
	uint64_t t = 0;
	for (unsigned char i = 0; i < film.GetTotalFrames(); )
		if (CurrTime() >= t) {
			t = CurrTime() + FRAME_DELAY; Vsync();
			BitmapClear(BitmapGetScreen(), BLACK_COLOR);
			film.DisplayFrame(BitmapGetScreen(), at, i++);
		}
}