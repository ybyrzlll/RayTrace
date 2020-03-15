#pragma once
#include "mMath.h"


class Camera {
public:
	Vector3f pos;
	Vector3f vpn;
	Vector3f up;
	float nearZ, laterialAngle, verticalAngle;

	Vector3f rightDir, upDir, pos_begin, rightFactor, downFactor;

	void refresh() {
		float half_h = tan(verticalAngle) * nearZ;
		float half_w = tan(laterialAngle) * nearZ;

		rightDir = (up.crossProduct(vpn)).normalized();
		upDir = (vpn.crossProduct(rightDir)).normalized();
		pos_begin = pos + vpn.normalized() * nearZ
			+ upDir * half_h - rightDir * half_w;
		rightFactor = rightDir * half_w * 2;
		downFactor = upDir * half_h * 2;
	}
};