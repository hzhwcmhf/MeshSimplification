#pragma once

class Point
{
public:
	union {
		struct { /*数组索引*/double p[3]; };
		struct { /*变量索引*/double x, y, z; };
	};

	Point() {}
	Point(double _x, double _y, double _z) :
		x(_x), y(_y), z(_z)
	{}

	Point operator+(const Point &b) const
	{
		return Point{ x + b.x, y + b.y, z + b.z };
	}
	Point& operator+=(const Point &b)
	{
		x += b.x, y += b.y, z += b.z;
		return *this;
	}
	Point operator-(const Point &b) const
	{
		return Point{ x - b.x, y - b.y, z - b.z };
	}
	Point operator-() const
	{
		return Point(-x, -y, -z);
	}
	Point& operator-=(const Point &b)
	{
		x -= b.x, y -= b.y, z -= b.z;
		return *this;
	}
	Point operator*(double p) const
	{
		return Point{ p*x, p*y, p*z };
	}
	Point& operator*=(const Point &p)
	{
		x *= p.x, y *= p.y, z *= p.z;
		return *this;
	}
	Point& operator*=(double p)
	{
		x *= p, y *= p, z *= p;
		return *this;
	}
	Point& operator/=(double p)
	{
		x /= p, y /= p, z /= p;
		return *this;
	}
	friend Point operator*(double p, const Point &x)
	{
		return Point{ p*x.x, p*x.y, p*x.z };
	}
	Point operator/(double p) const
	{
		return Point{ x / p,  y / p,  z / p };
	}
	friend double dot(const Point &a, const Point &b)
	{
		return a.x * b.x + a.y*b.y + a.z*b.z;
	}
	friend Point cross(const Point &a, const Point &b)
	{
		return Point{
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x
		};
	}
	friend Point cross(const Point &a, const Point &o, const Point &b)
	{
		return cross(a - o, b - o);
	}
	friend double norm(const Point &a)
	{
		return a.x*a.x + a.y*a.y + a.z*a.z;
	}
	friend double abs(const Point &a)
	{
		return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	}
};