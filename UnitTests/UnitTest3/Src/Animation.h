#pragma ocne

#include <vector>
#include <map>
#include "app.h"
#include "Bitmap.h"
#include "Defines.h"

namespace app {
	class Animation {
	protected:
		std::string id;
	public:
		Animation(const std::string&);

		virtual ~Animation();
		const std::string& GetId(void);
		void SetId(const std::string&);
		virtual Animation* Clone(void) const = 0;
	};

	class MovingAnimation: public Animation{
	protected:
		unsigned reps = 1; // 0=forever
		int dx = 0, dy = 0;
		unsigned delay = 0;
	public:
		MovingAnimation(const std::string& _id, unsigned _reps, int _dx, int _dy, unsigned _delay);

		using Me= MovingAnimation;
		int GetDx(void) const;
		Me& SetDx(int v);
		int GetDy(void) const;
		Me& SetDy(int v);
		unsigned GetDelay(void) const;
		Me& SetDelay(unsigned v);
		unsigned GetReps(void) const;
		Me& SetReps(unsigned n);
		bool IsForever(void) const;
		Me& SetForever(void);
		Animation* Clone(void) const override;
	};

	class FrameRangeAnimation: public MovingAnimation {
	protected:
		unsigned start = 0, end = 0;
	public:
		FrameRangeAnimation(const std::string& _id, unsigned s, unsigned e, unsigned r, int dx, int dy, int d);

		using Me = FrameRangeAnimation;
		unsigned GetStartFrame(void) const;
		Me& SetStartFrame(unsigned v);
		unsigned GetEndFrame(void) const;
		Me& SetEndFrame(unsigned v);
		Animation* Clone(void) const override;
	};

	class FrameListAnimation: public MovingAnimation {
	public:
		using Frames = std::vector<unsigned>;
	protected:
		Frames frames;
	public:
		FrameListAnimation(const std::string& _id, const Frames& _frames, unsigned r, int dx, int dy, unsigned d, bool c);

		const Frames& GetFrames(void) const;
		void SetFrames(const Frames& f);
		Animation* Clone(void) const override;
	};

	struct PathEntry {
		int dx = 0, dy = 0;
		unsigned frame = 0;
		unsigned delay = 0;
		PathEntry(void) = default;
		PathEntry(const PathEntry&) = default;
	};
	
	class MovingPathAnimation: public Animation{
	public:
		using Path = std::vector<PathEntry>;
	private:
		Path path;
	public:
		const Path& GetPath(void) const {
			return path;
		}
		void SetPath(const Path& p) {
			path = p;
		}
		Animation* Clone(void) const override {
			return new MovingPathAnimation(id, path);
		}
		MovingPathAnimation(const std::string& _id, const Path& _path): path(_path), Animation(id) {}
	};

	class FlashAnimation: public Animation {
	private:
		unsigned repetitions = 0;
		unsigned hideDelay = 0;
		unsigned showDelay = 0;
	public:
		using Me = FlashAnimation;

		FlashAnimation(const std::string& _id, unsigned n, unsigned show, unsigned hide);

		Me& SetRepetitions(unsigned n);
		unsigned GetRepetitions(void) const;
		Me& SetHideDeay(unsigned d);
		unsigned GetHideDeay(void) const;
		Me& SetShowDeay(unsigned d);
		unsigned GetShowDeay(void) const;
		Animation* Clone(void) const override;
		
	};

	struct ScrollEntry {
		int dx = 0;
		int dy = 0;
		unsigned delay = 0;
	};

	class ScrollAnimation: public Animation{
	public:
		using Scroll = std::vector<ScrollEntry>;
	private:
		Scroll scroll;
	public:
		ScrollAnimation(const std::string& _id, const Scroll& _scroll);

		const Scroll& GetScroll(void) const;
		void SetScroll(const Scroll& p);
		Animation* Clone(void) const override;
	};

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
		Bitmap Load(const std::string& path);
		void CleanUp(void);
	};

	void Animate(AnimationFilm& film, const Point at);
}