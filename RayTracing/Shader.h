#pragma once
#include <iostream>
#include "mMath.h"
#include "light.h"
#include "Intersect.h"
#include "Global.h"

static UINT32 v3f_2_UINT32(Vector3f in) {
	in.x = clamp(in.x, 0.0f, 255.0f);
	in.y = clamp(in.y, 0.0f, 255.0f);
	in.z = clamp(in.z, 0.0f, 255.0f);
	return ((int)in.x << 16) + ((int)in.y << 8) + in.z;
}

static class Lambert {
public:
	static Vector3f shade(const Ray& ray, const Intersection& intersection,const Light* light) {
		float temp = max(0.0,ray.dir.dot(-intersection.normal));
		return light->color * temp;
	}
};

