#pragma once
#include "mesh.h"
#include "Matarial.h"
#include "mMath.h"
#include "Shader.h"

struct Obj {
public:
	Vector3f transform;
	Mesh* mesh;
	Matarial* matarial;
	BaseShader* shader;
	Obj() {};
};
