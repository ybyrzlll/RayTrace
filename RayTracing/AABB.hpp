#pragma once
#include "BoundingBox.hpp"
class AABB : public BoundingBox {
public:
	Vector3f minP, maxP;

	AABB(Vector3f minp, Vector3f maxp) :minP(minp), maxP(maxp) {}

	bool intersect(const Ray &ray) {
		
		if (ray.dir.x> 0.0001 || ray.dir.x< -0.0001) {
			double t1 = (minP.x - ray.pos.x) / ray.dir.x, t2 = (maxP.x - ray.pos.x) / ray.dir.x;
			t1 = min(max(t1, 0), max(t2, 0));
			double y = ray.pos.y + t1 * ray.dir.y, 
				z = ray.pos.z + t1 * ray.dir.z;
			if (minP.y <= y && y <= maxP.y && minP.z <= z && z <= maxP.z)
				return true;
		}
		if (ray.dir.y> 0.0001 || ray.dir.y< -0.0001) {
			double t1 = (minP.y - ray.pos.y) / ray.dir.y, t2 = (maxP.y - ray.pos.y) / ray.dir.y;
			t1 = min(max(t1, 0), max(t2, 0));
			double x = ray.pos.x + t1 * ray.dir.x, 
				z = ray.pos.z + t1 * ray.dir.z;
			if (minP.x <= x && x <= maxP.x && minP.z <= z && z <= maxP.z)
				return true;
		}
		if (ray.dir.z> 0.0001 || ray.dir.z< -0.0001) {
			double t1 = (minP.z - ray.pos.z) / ray.dir.z, t2 = (maxP.z - ray.pos.z) / ray.dir.z;
			t1 = min(max(t1, 0), max(t2, 0));
			double y = ray.pos.y + t1 * ray.dir.y, 
				x = ray.pos.x + t1 * ray.dir.x;
			if (minP.y <= y && y <= maxP.y && minP.x <= x && x <= maxP.x)
				return true;
		}
		return false;
	}
};