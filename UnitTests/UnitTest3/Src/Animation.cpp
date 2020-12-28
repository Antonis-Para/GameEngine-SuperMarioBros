#include "Animation.h"

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

app::BitmapLoader::BitmapLoader(void) {}
app::BitmapLoader::~BitmapLoader() {
	CleanUp();
}

Bitmap app::BitmapLoader::GetBitmap(const std::string& path) const {
	auto i = bitmaps.find(path);
	return i != bitmaps.end() ? i->second : nullptr;
}

Bitmap app::BitmapLoader::BitmapLoad(const std::string& path) {
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

void app::Animate(AnimationFilm& film, const Point at) {
	uint64_t t = 0;
	for (unsigned char i = 0; i < film.GetTotalFrames(); )
		if (CurrTime() >= t) {
			t = CurrTime() + FRAME_DELAY; Vsync();
			BitmapClear(BitmapGetScreen(), BLACK_COLOR);
			film.DisplayFrame(BitmapGetScreen(), at, i++);
		}
}