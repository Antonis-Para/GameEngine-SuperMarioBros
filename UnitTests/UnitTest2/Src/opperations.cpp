#include "app.h"

bool operator<(const app::Color left, const app::Color right) {
	return left.a < right.a;
}