#pragma once
#include "mMath.h"
#include "Intersection.h"
struct Matarial {
	Vector3f color;
	float reflFactor;//������
	

	//�⻬��,͸����etc

	Vector3f brdf(Intersection inst, Vector3f in, Vector3f &out) {
		Vector3f color = { 0, 0, 0 };//������ɫ
		if (true) {
			out = reflection(inst.normal, in);
		}
		else {

		}
		return color;
	}
};