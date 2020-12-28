#include "Destruction.h"

// DestructionManager
auto app::DestructionManager::Get(void) -> DestructionManager& {
	return singleton;
}

void app::DestructionManager::Register(LatelyDestroyable* d) {
	assert(!d->IsAlive());
	dead.push_back(d);
}

void app::DestructionManager::Commit(void) {
	for (auto* d : dead)d->Delete();
	dead.clear();
}

// LatelyDestroyable
app::LatelyDestroyable::~LatelyDestroyable() {
	assert(dying);
}

bool app::LatelyDestroyable::IsAlive(void) const {
	return alive;
}

void app::LatelyDestroyable::Destroy(void) {
	if (alive) {
		alive = false;
		DestructionManager::Get().Register(this);
	}
}

void app::LatelyDestroyable::Delete(void) {
	assert(!dying);
	dying = true;
	delete this;
}

// Recycler
/*static T* app::Recycled::top_and_pop(void) {
	auto* x = recycler.top();
	recycler.pop();
	return x;
}

template<class... Types>
static T* app::Recycled::New(Types... args) {
	if (recycler.empty())
		return new T(args...); // automatic propagation of any arguments!
	else
		return new (top_and_pop()) T(args...);// reusing ...
}

void app::Recycled::Delete(void) {
	this->~T();
	recycler.push(this);
}*/