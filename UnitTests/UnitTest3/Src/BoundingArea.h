#pragma once

#include <list>

class BoundingBox;
class BoundingCircle;
class BoundingPolygon;

class BoundingArea {
public:
	virtual bool Intersects(const BoundingBox&) const = 0;
	virtual bool Intersects(const BoundingCircle&) const = 0;
	virtual bool Intersects(const BoundingPolygon&) const = 0;
	virtual bool In(unsigned, unsigned) const = 0;
	virtual bool Intersects(const BoundingArea&) const = 0;
	virtual BoundingArea* Clone(void) const = 0;
	~BoundingArea() {}
};

class BoundingBox: public BoundingArea {
private:
	unsigned x1, y1, x2, y2;
public:
	BoundingBox(unsigned, unsigned, unsigned, unsigned);

	virtual bool Intersects(const BoundingBox&) const override;
	virtual bool Intersects(const BoundingCircle&) const override;
	virtual bool Intersects(const BoundingPolygon&) const override;
	virtual bool In(unsigned, unsigned) const;
	virtual bool Intersects(const BoundingArea&) const;
	virtual BoundingArea* Clone(void) const;

	unsigned getMaxDiagonal(void) const;
	void getCenter(unsigned&, unsigned&) const;
};

class BoundingCircle : public BoundingArea {
private:
	unsigned x, y, r;
public:
	BoundingCircle(unsigned, unsigned, unsigned);

	virtual bool Intersects(const BoundingBox&) const override;
	virtual bool Intersects(const BoundingCircle&) const override;
	virtual bool Intersects(const BoundingPolygon&) const override;
	virtual bool In(unsigned, unsigned) const;
	virtual bool Intersects(const BoundingArea&) const;
	virtual BoundingArea* Clone(void) const;
};

class BoundingPolygon : public BoundingArea {
public:
	struct Point {
		unsigned x, y;
		Point(void) : x(0), y(0) {}
		Point(unsigned _x, unsigned _y) : x(_x), y(_y) {}
		Point(const Point& other) : x(other.x), y(other.y) {}
	};
	typedef std::list<Point> Polygon;
protected:
	Polygon points;
public:
	BoundingPolygon(const Polygon&);

	virtual bool Intersects(const BoundingBox&) const override;
	virtual bool Intersects(const BoundingCircle&) const override;
	virtual bool Intersects(const BoundingPolygon&) const override;
	virtual bool In(unsigned, unsigned) const;
	virtual bool Intersects(const BoundingArea&) const;
	virtual BoundingArea* Clone(void) const;
};