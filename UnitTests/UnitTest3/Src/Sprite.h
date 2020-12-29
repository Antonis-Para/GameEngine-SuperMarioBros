#pragma once

#include "app.h"
#include "Animation.h"

namespace app {
	class MotionQuantizer {
	public:
		using Mover = std::function<void(const Rect& r, int* dx, int* dy)>;
	protected:
		int horizMax = 0, vertMax = 0;
		Mover mover; // filters too
		bool used = false;
	public:
		MotionQuantizer(void) = default;
		MotionQuantizer(const MotionQuantizer&) = default;

		MotionQuantizer& SetRange(int h, int v);
		template<typename Tfunc>
		MotionQuantizer& SetMover(const Tfunc& f);
		void Move (const Rect& r, int* dx, int* dy);
		
	};
	
	class Clipper {
	public:
		using View = std::function<const Rect& (void)>;
	private:
		View view;
	public:
		Clipper(void) = default;
		Clipper(const Clipper&) = default;

		template<typename Tfunc>
		Clipper& SetView(const Tfunc& f);
		bool Clip(const Rect& r, const Rect& dpyArea, Point* dpyPos, Rect* clippedBox) const;
		
	};
	class Sprite {
	public:
		using Mover = std::function<void(const Rect&, int* dx, int* dy)>;
	protected:
		unsigned char frameNo = 0;
		Rect frameBox;
		int x = 0, y = 0;
		bool isVisible = false;
		AnimationFilm* currFilm = nullptr;
		BoundingArea* boundingArea = nullptr;
		unsigned zorder = 0;
		std::string typeId, stateId;
		Mover mover;
		MotionQuantizer quantizer;
	public:
		Sprite(int _x, int _y, AnimationFilm* film, const std::string& _typeId = "");

		template<typename Tfunc>
		void SetMover(const Tfunc& f);
		const Rect GetBox(void) const;
		void Move(int dx, int dy);
		void SetPos(int _x, int _y);
		void SetZorder(unsigned z);
		unsigned GetZorder(void);
		void SetFrame(unsigned char i);
		unsigned char GetFrame(void) const;
		void SetBoundingArea(const BoundingArea& area);
		void SetBoundingArea(BoundingArea* area);
		auto GetBoundingArea(void) const -> const BoundingArea*;
		auto GetTypeId(void) -> const std::string&;
		void SetVisibility(bool v);
		bool IsVisible(void) const;
		bool CollisionCheck(const Sprite* s) const;
		void Display(Bitmap dest, const Rect& dpyArea, const Clipper& clipper) const;
	};

	template<typename Tnum>
	int number_sign(Tnum x);
	template<class T>
	bool clip_rect(T x, T y, T w, T h, T wx, T wy, T ww, T wh, T* cx, T* cy, T* cw, T* ch);
	bool clip_rect(const Rect& r, const Rect& area, Rect* result);
}