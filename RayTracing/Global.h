#pragma once
#include "mesh.h"
#include "light.h"
#define reflFactor 0.2//��������
#define reflTimes 1//׷�ټ�ӹⷴ��Ĵ���
#define Camera_Speed 0.1;
#define Camera_RotateSpeed 0.02;

Mesh cuboid, plane;
vector<Light*> lights;
vector<Mesh*> meshs;