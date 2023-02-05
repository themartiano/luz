#pragma once

#include "Vector3.hpp"
#include <random>

class Random
{
	public:
		Random();
		Random(int_fast32_t seed);
		void seed(int_fast32_t newSeed);
		double doubleFloat(void);
		double doubleFloat(double min, double max);
		int integer(void);
		int integer(int min, int max);
		Vector3 cosineDirection(void);
		Vector3 pointInsideUnitSphere(void);
		Vector3 pointInsideUnitDisk(void);

	private:
		std::mt19937 _engine;
};

extern Random randomEngine;
