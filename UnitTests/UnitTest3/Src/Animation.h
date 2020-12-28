#pragma ocne

#include <vector>
#include <map>
#include "app.h"
#include "Bitmap.h"
#include "Defines.h"

namespace app {
	class AnimationFilm {
		std::vector<Rect> boxes;
		Bitmap bitmap = nullptr;
		std::string id;
	public:
		AnimationFilm(const std::string& _id) : id(_id) {}
		AnimationFilm(Bitmap, const std::vector<Rect>&, const std::string&);
		unsigned char GetTotalFrames(void) const;
		Bitmap GetBitmap(void) const;
		auto GetId(void) const -> const std::string&;
		const Rect& GetFrameBox(unsigned char frameNo) const;
		void DisplayFrame(Bitmap dest, const Point& at, unsigned char frameNo) const;
		void SetBitmap(Bitmap b);
		void Append(const Rect& r);
	};

	class AnimationFilmHolder {
		using Films = std::map<std::string, AnimationFilm*>;
		Films films;
		BitmapLoader bitmaps;// only for loading of film bitmaps
		static AnimationFilmHolder holder;// singleton
		AnimationFilmHolder(void) {}
		~AnimationFilmHolder() { CleanUp(); }
	public:
		static auto Get(void) -> const AnimationFilmHolder&;
		// TODO(4u): set a parsing functor implemented externally to the class
		static int ParseEntry( // -1=error, 0=ended gracefully, else #chars read
			int startPos, const std::string& text, std::string& id, std::string& path, std::vector<Rect>& rects);
		void LoadAll(const std::string& text);
		void CleanUp(void);
		auto GetFilm(const std::string& id) -> const AnimationFilm* const;
	};

	class BitmapLoader {
	private:
		using Bitmaps = std::map<std::string, Bitmap>;
		Bitmaps bitmaps;
		Bitmap GetBitmap(const std::string& path) const;
	public:
		BitmapLoader(void);
		~BitmapLoader();
		Bitmap BitmapLoad(const std::string& path);
		void CleanUp(void);
	};

	void Animate(AnimationFilm& film, const Point at);
}