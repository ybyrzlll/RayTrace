#pragma once
#include"mMath.h"
class BoundingBox {
public:
	virtual bool intersect(const Ray &ray) = 0;
};