#pragma once

#include "Vector3.hpp"
#include "Materials/Material.hpp"
#include <memory>

class	Material;

struct HitRecord
{
	double		t0 = 0.0;
	double		t1 = 0.0;
	Vector3		position;
	Vector3		normal;
	std::shared_ptr<Material>	material;
};
