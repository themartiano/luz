#pragma once

#include "Vector3.hpp"
#include <random>

#define RANDOM_ENGINE std::minstd_rand // faster than std::mt19937
// test:
// linear_congruential_engine
// subtract_with_carry_engine
// discard_block_engine
// independent_bits_engine

class Random
{
	public:
		Random();
		Random(int_fast32_t seed);
		void seed(int_fast32_t newSeed);
		double doubleFloat(void);
		double doubleFloat(double min, double max);
		unsigned int integer(void);
		unsigned int integer(int min, int max);
		Vector3 cosineDirection(void);
		Vector3 pointInsideUnitSphere(void);
		Vector3 pointInsideUnitDisk(void);

	private:
		RANDOM_ENGINE _engine;
};

extern Random randomEngine;
