#pragma once
#include "mMath.h"
#include "Intersection.h"
struct Matarial {
	Vector3f color;
	float reflFactor, diffFactor, refrFactor;//反射率, 漫反射率, 折射率
	int type;//1 reflection  2 diffuse  3 refract

	//光滑度,透明度etc

	//Vector3f brdf(Intersection inst, Vector3f in, Vector3f &out) {
	//	Vector3f color = { 0, 0, 0 };//纹理颜色
	//	if (true) {
	//		out = reflection(inst.normal, in);
	//	}
	//	else {

	//	}
	//	return color;
	//}
};