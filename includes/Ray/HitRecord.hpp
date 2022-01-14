#pragma once

#include "Vector3.hpp"
#include "Materials/Material.hpp"

struct HitRecord
{
	double		t0 = 0.0;
	double		t1 = 0.0;
	Vector3		position = Vector3(0.0, 0.0, 0.0);
	Vector3		normal = Vector3(0.0, 0.0, 0.0);
	Material	material = Material();
};
