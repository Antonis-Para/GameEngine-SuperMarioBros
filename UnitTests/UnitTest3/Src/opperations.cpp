#include "app.h"

bool operator<(const Color left, const Color right) {
	return left.a < right.a;
}