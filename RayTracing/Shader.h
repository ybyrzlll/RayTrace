#pragma once
#include <iostream>
#include "mMath.h"
#include "light.h"
#include "Intersect.h"

static UINT32 v3f_2_UINT32(Vector3f in) {
	in.x = clamp(in.x, 0.0f, 255.0f);
	in.y = clamp(in.y, 0.0f, 255.0f);
	in.z = clamp(in.z, 0.0f, 255.0f);
	return ((int)in.x << 16) + ((int)in.y << 8) + in.z;
}

static class Lambert {
public:
	static UINT32 shade(const Ray& ray, Intersect& intersect, Light* light) {
		float temp = ray.dir.dot(intersect.normal);
		return v3f_2_UINT32(light->color * temp);
	}
};

