#pragma once
#define reflTimes 2//追踪间接光反射的次数
#define Camera_Speed 0.1;
#define Camera_RotateSpeed 0.02;
int window_width = 400, window_height = 300;

vector<Light*> lights;
vector<Obj*> objs;