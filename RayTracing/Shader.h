#pragma once
#include <iostream>
#include "light.h"
#include "Matarial.h"
#include "Intersection.h"

class BaseShader {
public:
	virtual ~BaseShader() {};
	//virtual Vector3f ashade(const Ray& ray, const Intersection& intersection, const Light* light) = 0;
	virtual Vector3f shade(const Ray& ray, const Intersection& intersection, const Light* light, const Matarial* mt) = 0;
};

class Lambert : public BaseShader {
public:
	Vector3f rgb = { 255, 255, 255 };
	/*Vector3f ashade(const Ray& ray, const Intersection& intersection, const Light* light) override {
		float temp = max(0.0, ray.dir.dot(-intersection.normal));
		return light->color * temp;
	}*/
	Vector3f shade(const Ray& ray, const Intersection& intersection, const Light* light, const Matarial* mt) override {
		float temp = max(0.0, ray.dir.dot(-intersection.normal));
		return light->color * mt->color * temp;//Vector3f(256, 0, 0.);
	}
};

