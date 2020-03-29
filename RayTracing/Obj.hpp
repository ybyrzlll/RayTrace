#pragma once
#include "mesh.h"
#include "Matarial.h"
#include "mMath.h"
#include "Shader.h"
#include "AABB.hpp"
#include "matrix.h"

struct Obj {
public:
	Matrix4 transform, rotation;
	Mesh *mesh;
	Matarial* matarial;
	BaseShader* shader;

	BoundingBox *boundingBox;
	
	Obj() {
		for (int i = 0; i < 4; i++) {
			transform.m[i][i] = 1;
			rotation.m[i][i] = 1;
		}
		boundingBox = nullptr;
	};
	~Obj() {};

	Vector3f emission;
	boolean self_luminous() {
		return emission.x > 0 || emission.y > 0 || emission.z > 0;
	};//光源或自发光

	void buildAABB() {
		Vector3f maxP, minP;
		maxP.x = maxP.y = maxP.z = INT_MIN;
		minP.x = minP.y = minP.z = INT_MAX;
		for (auto p : mesh->vertices) {
			p = matrix_mul(this->rotation, p);
			p = matrix_mul(this->transform, p);
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


	void zoom(double ratio) {
		for (int i = 0; i < 4; i++)
			transform.m[i][i] *= ratio;
	}

	void rotate(Vector3f dir, float theta) {
		matrix_set_rotate(&rotation, dir, theta);
	}

	void translate(Vector3f dir) {
		transform.m[3][0] += dir.x;
		transform.m[3][1] += dir.y;
		transform.m[3][2] += dir.z;
	}
};
