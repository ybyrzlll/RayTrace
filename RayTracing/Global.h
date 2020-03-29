#pragma once
#define EXP (1.0001);
#define recurTimes 8//追踪的次数
#define screenSamples 40//屏幕采样个数
#define Camera_Speed 0.1;
#define Camera_RotateSpeed 0.02;
int window_width = 400, window_height = 300;

vector<Light*> lights;
vector<Obj*> objs;