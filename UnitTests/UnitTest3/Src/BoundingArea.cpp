#include "BoundingArea.h"

// BoundingBox
bool BoundingBox::Intersects(const BoundingBox &box) const {
	return !(
		box.x2 < x1 ||
		x2 < box.x1 ||
		box.y2 < y1 ||
		y2 < box.y1
		);
}

bool BoundingBox::Intersects(const BoundingCircle &circle) const {
	return circle.Intersects(*this);
}

bool BoundingBox::Intersects(const BoundingPolygon &poly) const {
	BoundingPolygon::Polygon points;
	points.push_back(BoundingPolygon::Point(x1, y1));
	points.push_back(BoundingPolygon::Point(x2, y2));
	BoundingPolygon selfPoly(points);

	return poly.Intersects(selfPoly);
}

bool BoundingBox::In(unsigned x, unsigned y) const {
	return x1 <= x && x <= x2 && y1 <= y && y <= y2;
}

BoundingBox::BoundingBox(unsigned _x1, unsigned _y1, unsigned _x2, unsigned _y2) :
	x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}

bool BoundingBox::Intersects(const BoundingArea &area) const {
	return area.Intersects(*this);
}

BoundingArea* BoundingBox::Clone(void) const {
	return new BoundingBox(x1, y1, x2, y2);
}

unsigned BoundingBox::getMaxDiagonal(void) const {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void BoundingBox::getCenter(unsigned &cx, unsigned &cy) const {
	cx = abs((long)(x2 - x1));
	cy = abs((long)(y2 - y1));
}

// BoundingCircle
bool BoundingCircle::Intersects(const BoundingBox& box) const {
	unsigned box_cx, box_cy;
	box.getCenter(box_cx, box_cy);
	unsigned distance = sqrt((x - box_cx) * (x - box_cx) + (y - box_cy) * (y - box_cy));
	if (distance > (r + box.getMaxDiagonal() / 2))
		return false;
	else {
		// TODO: Lecture 7, Slide 26
		return true;
	}
}

bool BoundingCircle::Intersects(const BoundingCircle& circle) const {
	return circle.Intersects(*this);
}

bool BoundingCircle::Intersects(const BoundingPolygon& poly) const {
	// same as BoundingBox but for N lines
	return false;
}

bool BoundingCircle::In(unsigned _x, unsigned _y) const {
	return sqrt((x - _x) * (x - _x) + (y - _y) * (y - _y)) <= r;
}

BoundingCircle::BoundingCircle(unsigned _x, unsigned _y, unsigned _r) :
	x(_x), y(_y), r(_r){}

bool BoundingCircle::Intersects(const BoundingArea& area) const {
	return area.Intersects(*this);
}

BoundingArea* BoundingCircle::Clone(void) const {
	return new BoundingCircle(x, y, r);
}

// BoundingPolygon
bool BoundingPolygon::Intersects(const BoundingPolygon& poly) const {
	// TODO
	return false;
}

bool BoundingPolygon::Intersects(const BoundingCircle& circle) const {
	// TODO
	return false;
}

bool BoundingPolygon::Intersects(const BoundingBox& box) const {
	// TODO
	return false;
}

bool BoundingPolygon::In(unsigned _x, unsigned _y) const {
	// TODO
	return false;
}

BoundingPolygon::BoundingPolygon(const Polygon &_points) :
	points(_points) {}

bool BoundingPolygon::Intersects(const BoundingArea& area) const {
	return area.Intersects(*this);
}

BoundingArea* BoundingPolygon::Clone(void) const {
	return new BoundingPolygon(points);
}