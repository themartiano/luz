#pragma once

#include "Vector3.hpp"
#include <ctime>

namespace	Random
{
	void			setSeed(unsigned int newSeed);
	unsigned int	getSeed(void);
	double			doubleFloat(void);
	double			doubleFloat(double min, double max);
	int				integer(void);
	int				integer(int min, int max);
	Vector3			cosineDirection(void);
	Vector3			pointInsideUnitSphere(void);
	Vector3			pointInsideUnitDisk(void);

	namespace {
		unsigned int	seed;
		bool			isSeeded = false;
	}
}
