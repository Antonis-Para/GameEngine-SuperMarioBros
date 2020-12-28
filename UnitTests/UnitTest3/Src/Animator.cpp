#include "Animator.h"

// Animator
app::Animator::Animator(void) {
	AnimatorManager::GetSingleton().Register(this);
}

app::Animator::~Animator(void) {
	AnimatorManager::GetSingleton().Cancel(this);
}

void app::Animator::NotifyStopped(void) {
	AnimatorManager::GetSingleton().MarkAsSuspended(this);
	if (onFinish)
		(onFinish)(this);
}

void app::Animator::NotifyStarted(void) {
	AnimatorManager::GetSingleton().MarkAsRunning(this);
	if (onStart)
		(onStart)(this);
}

void app::Animator::NotifyAction(const Animation& anim) {
	if (onAction)
		(onAction)(this, anim);
}

void app::Animator::Finish(bool isForced) {
	if (!HasFinished()) {
		state = isForced ? ANIMATOR_STOPPED : ANIMATOR_FINISHED; NotifyStopped();
	}
}

bool app::Animator::HasFinished(void) const {
	return state != ANIMATOR_RUNNING;
}

template<typename Tfunc>
void app::Animator::SetOnFinish(const Tfunc& f) {
	onFinish = f;
}

template<typename Tfunc>
void app::Animator::SetOnStart(const Tfunc& f) {
	onStart = f;
}

template<typename Tfunc>
void app::Animator::SetOnAction(const Tfunc& f) {
	onAction = f;
}

void app::Animator::Stop(void) {
	Finish(true);
}

void app::Animator::TimeShift(timestamp_t offset) {
	lastTime += offset;
}

// MovingAnimator
void app::MovingAnimator::Progress(timestamp_t currTime) {
	while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
		lastTime += anim->GetDelay();
		NotifyAction(*anim);
		if (!anim->IsForever() && ++currRep == anim->GetReps()) {
			state = ANIMATOR_FINISHED;
			NotifyStopped();
		}
	}
}

auto app::MovingAnimator::GetAnim(void) const -> const MovingAnimation& {
	return *anim;
}

void app::MovingAnimator::Start(MovingAnimation* a, timestamp_t t) {
	anim = a;
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currRep = 0;
	NotifyStarted();
}

// FrameRangeAnimator
void app::FrameRangeAnimator::Progress(timestamp_t currTime) {
	while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
		if (currFrame == anim->GetEndFrame()) {
			assert(anim->IsForever() || currRep < anim->GetReps());
			currFrame = anim->GetStartFrame(); // flip to start
		}
		else
			++currFrame;
		lastTime += anim->GetDelay();
		NotifyAction(*anim);
		if (currFrame == anim->GetEndFrame())
			if (!anim->IsForever() && ++currRep == anim->GetReps()) {
				state = ANIMATOR_FINISHED; NotifyStopped();
				return;
			}
	}
}

unsigned app::FrameRangeAnimator::GetCurrFrame(void) const {
	return currFrame;
}

unsigned app::FrameRangeAnimator::GetCurrRep(void) const {
	return currRep;
}

void app::FrameRangeAnimator::Start(FrameRangeAnimation* a, timestamp_t t) {
	anim = a;
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currFrame = anim->GetStartFrame();
	currRep = 0;
	NotifyStarted();
	NotifyAction(*anim);
}

// TickAnimator
void app::TickAnimator::Progress(timestamp_t currTime) {
	if (!anim->IsDiscrete()) {
		elapsedTime = currTime - lastTime;
		lastTime = currTime;
		NotifyAction(*anim);
	}
	else
		while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
			lastTime += anim->GetDelay();
			NotifyAction(*anim);
			if (!anim->IsForever() && ++currRep == anim->GetReps()) {
				state = ANIMATOR_FINISHED;
				NotifyStopped();
				return;
			}
		}
}

unsigned app::TickAnimator::GetCurrRep(void) const {
	return currRep;
}

unsigned app::TickAnimator::GetElapsedTime(void) const {
	return elapsedTime;
}

float app::TickAnimator::GetElapsedTimeNormalised(void) const {
	return float(elapsedTime) / float(anim->GetDelay());
}

void app::TickAnimator::Start(const TickAnimation& a, timestamp_t t) {
	anim = (TickAnimation*)a.Clone();
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currRep = 0;
	elapsedTime = 0;
	NotifyStarted();
}

// AnimatorManager
void app::AnimatorManager::Register(Animator* a) {
	assert(a->HasFinished());
	suspended.insert(a);
}

void app::AnimatorManager::Cancel(Animator* a) {
	assert(a->HasFinished());
	suspended.erase(a);
}

void app::AnimatorManager::MarkAsRunning(Animator* a) {
	assert(!a->HasFinished());
	suspended.erase(a);
	running.insert(a);
}

void app::AnimatorManager::MarkAsSuspended(Animator* a) {
	assert(a->HasFinished());
	running.erase(a);
	suspended.insert(a);
}

void app::AnimatorManager::Progress(timestamp_t currTime) {
	auto copied(running);
	for (auto* a : copied)
		a->Progress(currTime);
}

auto app::AnimatorManager::GetSingleton(void) -> AnimatorManager& {
	return singleton;
}

auto app::AnimatorManager::GetSingletonConst(void) -> const AnimatorManager& {
	return singleton;
}