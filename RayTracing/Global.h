#pragma once
#include "mesh.h"
#include "light.h"
#define reflFactor 0.2//反射因子
#define reflTimes 1//追踪间接光反射的次数
#define Camera_Speed 0.1;
#define Camera_RotateSpeed 0.02;

Mesh cuboid, plane;
vector<Light*> lights;
vector<Mesh*> meshs;