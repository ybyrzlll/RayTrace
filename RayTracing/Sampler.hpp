#pragma once
#include<utility>
using namespace std;

class Sampler {
public:
	Sampler() {}
	virtual double get1D() = 0;
	virtual pair<double, double> get2D() = 0;
	~Sampler() {}
};