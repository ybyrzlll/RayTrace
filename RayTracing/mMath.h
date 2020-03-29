#pragma once
#include <math.h>
#define pi 3.1415926535
#define c2z 0.00001

template<typename T>
struct Vector3 {
	union {
		T data[3];
		struct {
			T x;
			T y;
			T z;
		};
	};
	T w;
	Vector3() {};
	Vector3(T val) :x(val), y(val), z(val), w(1) {};
	Vector3(T a, T b, T c) : x(a), y(b), z(c), w(1) {};

	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}
	Vector3 operator + (const Vector3 rhs)const
	{
		return Vector3(rhs.x + x, rhs.y + y, rhs.z + z);
	}
	Vector3 operator - (const Vector3 rhs)const
	{
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}
	Vector3 operator * (const Vector3 rhs)const
	{
		return Vector3(rhs.x * x, rhs.y * y, rhs.z * z);
	}
	Vector3& operator += (const Vector3 rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	

	Vector3 operator * (const T a) const
	{
		return Vector3(x * a, y * a, z * a);
	}

	Vector3& operator *= (const T a)
	{
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}

	T dot(const Vector3 rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}
	Vector3 crossProduct(const Vector3 rhs) const
	{
		return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}

	T length() const
	{
		return sqrt(x * x + y * y + z * z);
	}
	Vector3& normalized()
	{
		T len = length();
		T factor = 1.0f / len;
		x *= factor;
		y *= factor;
		z *= factor;
		return *this;
	}
	boolean operator==(Vector3 rhs) const {
		if (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z)
			return true;
		else
			return false;
	}
};

typedef Vector3<float> Vector3f;
typedef Vector3<int> Vector3i;


struct Ray {
	Vector3f pos;
	Vector3f dir;
	Ray() {};
	Ray(Vector3f p, Vector3f d) :pos(p), dir(d) {};
};

template<typename T>
static T clamp(T x, T min, T max) { return (x < min) ? min : ((x > max) ? max : x); }

static Vector3f reflection(const Vector3f &normal, const Vector3f &in) {
	Vector3f N = normal * ((in.dot(normal)) / normal.length());
	Vector3f V = -N + in;
	return (V * 2 - in);//.normalized();
}

static float rand_0_1() {
	return (float)rand() / RAND_MAX;
}

static UINT32 v3f_2_UINT32(Vector3f in) {
	in.x = clamp(in.x, 0.0f, 255.0f);
	in.y = clamp(in.y, 0.0f, 255.0f);
	in.z = clamp(in.z, 0.0f, 255.0f);
	return ((int)in.x << 16) + ((int)in.y << 8) + in.z;
}
