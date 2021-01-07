#include "Animator.h"

// Animator
Animator::Animator(void) {
	AnimatorManager::GetSingleton().Register(this);
}

Animator::~Animator(void) {
	AnimatorManager::GetSingleton().Cancel(this);
}

void Animator::NotifyStopped(void) {
	AnimatorManager::GetSingleton().MarkAsSuspended(this);
	if (onFinish)
		(onFinish)(this);
}

void Animator::NotifyStarted(void) {
	AnimatorManager::GetSingleton().MarkAsRunning(this);
	if (onStart)
		(onStart)(this);
}

void Animator::NotifyAction(const Animation& anim) {
	if (onAction)
		(onAction)(this, anim);
}

void Animator::Finish(bool isForced) {
	if (!HasFinished()) {
		state = isForced ? ANIMATOR_STOPPED : ANIMATOR_FINISHED; 
		NotifyStopped();
	}
}

bool Animator::HasFinished(void) const {
	return state != ANIMATOR_RUNNING;
}

void Animator::Stop(void) {
	Finish(true);
}

void Animator::TimeShift(timestamp_t offset) {
	lastTime += offset;
}

// MovingAnimator
void MovingAnimator::Progress(timestamp_t currTime) {
	while (currTime > lastTime && (currTime - lastTime) >= anim->GetDelay()) {
		lastTime += anim->GetDelay();
		NotifyAction(*anim);
		if (!anim->IsForever() && ++currRep == anim->GetReps()) {
			state = ANIMATOR_FINISHED;
			NotifyStopped();
		}
	}
}

auto MovingAnimator::GetAnim(void) const -> const MovingAnimation& {
	return *anim;
}

void MovingAnimator::Start(MovingAnimation* a, timestamp_t t) {
	anim = a;
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currRep = 0;
	//NotifyStarted();
}

/*void Sprite_MoveAction(Sprite* sprite, const MovingAnimation& anim) {
	sprite->Move(anim.GetDx(), anim.GetDy());
}

animator->SetOnAction([sprite](Animator* animator, constAnimation& anim) {
	Sprite_MoveAction(sprite, (constMovingAnimation&)anim);
	}
);*/

// FrameRangeAnimator
void FrameRangeAnimator::Progress(timestamp_t currTime) {
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

unsigned FrameRangeAnimator::GetCurrFrame(void) const {
	return currFrame;
}

unsigned FrameRangeAnimator::GetCurrRep(void) const {
	return currRep;
}

void FrameRangeAnimator::Start(FrameRangeAnimation* a, timestamp_t t) {
	anim = a;
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currFrame = anim->GetStartFrame();
	currRep = 0;
	NotifyStarted();
	NotifyAction(*anim);
}

// TickAnimator
void TickAnimator::Progress(timestamp_t currTime) {
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

unsigned TickAnimator::GetCurrRep(void) const {
	return currRep;
}

unsigned TickAnimator::GetElapsedTime(void) const {
	return elapsedTime;
}

float TickAnimator::GetElapsedTimeNormalised(void) const {
	return float(elapsedTime) / float(anim->GetDelay());
}

void TickAnimator::Start(const TickAnimation& a, timestamp_t t) {
	anim = (TickAnimation*)a.Clone();
	lastTime = t;
	state = ANIMATOR_RUNNING;
	currRep = 0;
	elapsedTime = 0;
	NotifyStarted();
}

// AnimatorManager
AnimatorManager AnimatorManager::singleton;

void AnimatorManager::Register(Animator* a) {
	assert(a->HasFinished());
	suspended.insert(a);
}

void AnimatorManager::Cancel(Animator* a) {
	assert(a->HasFinished());
	suspended.erase(a);
}

void AnimatorManager::MarkAsRunning(Animator* a) {
	assert(!a->HasFinished());
	suspended.erase(a);
	running.insert(a);
}

void AnimatorManager::MarkAsSuspended(Animator* a) {
	assert(a->HasFinished());
	running.erase(a);
	suspended.insert(a);
}

void AnimatorManager::Progress(timestamp_t currTime) {
	auto copied(running);
	for (auto* a : copied)
		a->Progress(currTime);
}

auto AnimatorManager::GetSingleton(void) -> AnimatorManager& {
	return singleton;
}

auto AnimatorManager::GetSingletonConst(void) -> const AnimatorManager& {
	return singleton;
}