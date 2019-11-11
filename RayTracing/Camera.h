#pragma once
#include "mMath.h"


class Camera {
public:
	Vector3f pos;
	Vector3f vpn;
	Vector3f up;
	float nearZ, laterialAngle, verticalAngle;
};