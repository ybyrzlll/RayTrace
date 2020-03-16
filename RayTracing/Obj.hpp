#pragma once
#include "mesh.h"
#include "Matarial.h"
#include "mMath.h"
#include "Shader.h"
#include "AABB.hpp"
#include "matrix.h"

struct Obj {
public:
	Matrix4 transform;
	Mesh *mesh;
	Matarial* matarial;
	BaseShader* shader;

	BoundingBox *boundingBox;
	
	Obj() {
		for (int i = 0; i < 4; i++)
			transform.m[i][i] = 1;
	};
	~Obj() {};

	void buildAABB() {
		Vector3f maxP, minP;
		maxP.x = maxP.y = maxP.z = INT_MIN;
		minP.x = minP.y = minP.z = INT_MAX;
		for (auto p : mesh->vertices) {
			if (p.x > maxP.x) maxP.x = p.x;
			if (p.x < minP.x) minP.x = p.x;
			if (p.y > maxP.y) maxP.y = p.y;
			if (p.y < minP.y) minP.y = p.y;
			if (p.z > maxP.z) maxP.z = p.z;
			if (p.z < minP.z) minP.z = p.z;
		}
		//showVector3(minP);
		//showVector3(maxP);
		this->boundingBox = new AABB(minP, maxP);
	}


	void Zoom(double ratio) {
		for (int i = 0; i < 4; i++)
			transform.m[i][i] *= ratio;
	}

	void Rotate() {

	}
};
