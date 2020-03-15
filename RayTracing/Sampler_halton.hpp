#pragma once

#include <mutex>

#include "Sampler.hpp"
class HaltonSampler : public Sampler {
public:
	HaltonSampler(int num1, int num2) {
		double factor = base_ = 1.0 / num2;
		value_ = 0.0;
		for (; num1 > 0; ) {
			value_ += factor * static_cast<double>(num1 % num2);
			num1 /= num2;
			factor *= base_;
		}
	}

	double next() {
		std::lock_guard<std::mutex> lock(mutex_);
		double r = 1.0 - value_ - 0.0000001;
		if (base_ < r) value_ += base_;
		else {
			double h = base_, hh;
			do {
				hh = h;
				h *= base_;
			} while (h >= r);
			value_ += hh + h - 1.0;
		}
		return value_;
	}

	double get1D() override {
		return next();
	}

	pair<double, double> get2D() override {
		double n = next();
		return pair<double, double>(n, n);
	}

private:
	std::mutex mutex_;
	double value_;
	double base_;
};