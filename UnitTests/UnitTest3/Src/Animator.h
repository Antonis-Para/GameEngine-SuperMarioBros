#pragma once

#include "app.h"
#include "Animation.h"
#include "Destruction.h"

enum animatorstate_t {
	ANIMATOR_FINISHED = 0, ANIMATOR_RUNNING = 1, ANIMATOR_STOPPED = 2
};

// may adopt this for animators in case we wish to Destroy() incallbacks
// and do not bother to have deleted pointers being used
class Animator: public LatelyDestroyable {
public:
	using OnFinish = std::function<void(Animator*)>;
	using OnStart = std::function<void(Animator*)>;
	using OnAction = std::function<void(Animator*, const Animation&)>;
protected:
	timestamp_t lastTime = 0;
	animatorstate_t state = ANIMATOR_FINISHED;
	OnFinish onFinish;
	OnStart onStart;
	OnAction onAction;

	void NotifyStopped(void);
	void NotifyStarted(void);
	void NotifyAction(const Animation&);
	void Finish(bool isForced = false);
public:
	Animator(void);
	Animator(const Animator&) = delete;
	Animator(Animator&&) = delete;
	virtual ~Animator();

	void Stop(void);
	bool HasFinished(void) const;

	virtual void TimeShift(timestamp_t offset);
	virtual void Progress(timestamp_t currTime) = 0;

	template<typename Tfunc>
	void SetOnFinish(const Tfunc& f);
	template<typename Tfunc>
	void SetOnStart(const Tfunc& f);
	template<typename Tfunc>
	void SetOnAction(const Tfunc& f);
};

template<typename Tfunc>
void Animator::SetOnFinish(const Tfunc& f) {
	onFinish = f;
}

template<typename Tfunc>
void Animator::SetOnStart(const Tfunc& f) {
	onStart = f;
}

template<typename Tfunc>
void Animator::SetOnAction(const Tfunc& f) {
	onAction = f;
}

class MovingAnimator : public Animator {
protected:
	MovingAnimation* anim = nullptr;
	unsigned currRep = 0; // animation state
public:
	MovingAnimator(void) = default;

	void Progress(timestamp_t currTime);
	auto GetAnim(void) const -> const MovingAnimation&;
	void Start(MovingAnimation* a, timestamp_t t);
};

/*void Sprite_MoveAction(Sprite* sprite, const MovingAnimation& anim) {
	sprite->Move(anim.GetDx(), anim.GetDy());
}

animator->SetOnAction([sprite](Animator* animator, constAnimation& anim) {
	Sprite_MoveAction(sprite, (constMovingAnimation&)anim);
	}
);*/

class FrameRangeAnimator : public Animator {
protected:
	FrameRangeAnimation* anim = nullptr;
	unsigned currFrame = 0; // animation state
	unsigned currRep = 0; // animation state
public:
	FrameRangeAnimator(void) = default;

	void Progress(timestamp_t currTime);
	unsigned GetCurrFrame(void) const;
	unsigned GetCurrRep(void) const;
	void Start(FrameRangeAnimation* a, timestamp_t t);
};


class TickAnimator: public Animator {
protected:
	TickAnimation *anim = nullptr;
	unsigned currRep = 0;
	unsigned elapsedTime = 0;
public:
	TickAnimator(void) = default;

	void Progress(timestamp_t currTime) override;
	unsigned GetCurrRep(void) const;
	unsigned GetElapsedTime(void) const;
	float GetElapsedTimeNormalised(void) const;
	void Start(const TickAnimation& a, timestamp_t t);
};

class AnimatorManager {
private:
	std::set<Animator*> running, suspended;
	static AnimatorManager singleton;
	AnimatorManager(void) = default;
	AnimatorManager(const AnimatorManager&) = delete;
	AnimatorManager(AnimatorManager&&) = delete;
public:
	void Register(Animator* a);
	void Cancel(Animator* a);
	void MarkAsRunning(Animator* a);
	void MarkAsSuspended(Animator* a);
	void Progress(timestamp_t currTime);
	static auto GetSingleton(void)->AnimatorManager&;
	static auto GetSingletonConst(void) -> const AnimatorManager&;
};