#include "Hittables/Cube.hpp"
#include "Vector3.hpp"
#include "Materials/Lambertian.hpp"
#include <algorithm>
#include <cmath>

/*
	Constructors
*/

// Constructs the Cube with default values (width, height, depth = 1.0)
Cube::Cube(void)
{
	this->_transform = Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0));
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_width = 1.0;
	this->_height = 1.0;
	this->_depth = 1.0;

	// Update faces
	_generateFaces();
}

// Constructs the Cube with custom values
Cube::Cube(Transform transform, double width, double height, double depth, std::shared_ptr<Material> material)
{
	this->_transform = transform;
	this->_width = width;
	this->_height = height;
	this->_depth = depth;
	this->_material = material;

	// Update faces
	_generateFaces();
}

// Sets the Cube's Transform
void	Cube::setTransform(Transform transform)
{
	this->_transform = transform;
	_generateFaces();
}

// Returns the Cube's material
Material*	Cube::getMaterial(void) const
{
	return (this->_material.get());
}

// Sets the Cube's Material
void	Cube::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
	_generateFaces();
}

// Sets the Cube's Width
void	Cube::setWidth(double width)
{
	this->_width = width;
	_generateFaces();
}

// Sets the Cube's Height
void	Cube::setHeight(double height)
{
	this->_height = height;
	_generateFaces();
}

// Sets the Cube's Depth
void	Cube::setDepth(double depth)
{
	this->_depth = depth;
	_generateFaces();
}

// Updates the Cube's faces
void	Cube::_generateFaces(void)
{
	Vector3 position = this->_transform.getPosition();

	this->_faces.clear();

	// Front
	this->_faces.push_back(Rectangle(Transform(position - Vector3(0.0, 0.0, this->_depth / 2.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

	// Back
	this->_faces.push_back(Rectangle(Transform(position + Vector3(0.0, 0.0, this->_depth / 2.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

	// Top
	this->_faces.push_back(Rectangle(Transform(position + Vector3(0.0, this->_height / 2.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_depth, this->_material));

	// Bottom
	this->_faces.push_back(Rectangle(Transform(position - Vector3(0.0, this->_height / 2.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_depth, this->_material));

	// Right
	this->_faces.push_back(Rectangle(Transform(position + Vector3(this->_width / 2.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_depth, this->_height, this->_material));

	// Left
	this->_faces.push_back(Rectangle(Transform(position - Vector3(this->_width / 2.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_depth, this->_height, this->_material));
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Cube::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	double currentClosestObject = t_max;
	bool anyHit = false;
	for (const Rectangle& rectangle : this->_faces)
	{
		if (rectangle.hit(ray, hitRecord, t_min, currentClosestObject))
		{
			currentClosestObject = hitRecord.t0;
			anyHit = true;
		}
	}

	return (anyHit);
}

bool	Cube::hitAny(Ray& ray, double t_min, double t_max) const
{
	for (const Rectangle& rectangle : this->_faces)
	{
		if (rectangle.hitAny(ray, t_min, t_max))
		{
			return (true);
		}
	}
	return (false);
}

bool	Cube::hitInterval(Ray& ray, double t_min, double t_max, double& t0, double& t1) const
{
	AABB boundingBox;

	if (!this->createBoundingBox(boundingBox))
	{
		return (false);
	}

	double intervalMin = t_min;
	double intervalMax = t_max;
	const Vector3& origin = ray.getOrigin();
	const Vector3& direction = ray.getDirection();
	const Vector3& minimum = boundingBox.getMinimum();
	const Vector3& maximum = boundingBox.getMaximum();

	for (int axis = 0; axis < 3; axis++)
	{
		if (direction[axis] == 0.0)
		{
			if (origin[axis] < minimum[axis] || origin[axis] > maximum[axis])
			{
				return (false);
			}
			continue;
		}

		const double inverseDirection = 1.0 / direction[axis];
		double axisT0 = (minimum[axis] - origin[axis]) * inverseDirection;
		double axisT1 = (maximum[axis] - origin[axis]) * inverseDirection;
		if (axisT0 > axisT1)
		{
			std::swap(axisT0, axisT1);
		}

		intervalMin = std::max(intervalMin, axisT0);
		intervalMax = std::min(intervalMax, axisT1);
		if (intervalMin >= intervalMax)
		{
			return (false);
		}
	}

	t0 = intervalMin;
	t1 = intervalMax;
	return (true);
}

// Creates an AABB / bounding box for this Cube
bool	Cube::createBoundingBox(AABB& outputBoundingBox) const
{
	const Vector3 position = this->_transform.getPosition();

	outputBoundingBox = AABB(
		Vector3(
			position.getX() - this->_width / 2.0,
			position.getY() - this->_height / 2.0,
			position.getZ() - this->_depth / 2.0
		),
		Vector3(
			position.getX() + this->_width / 2.0,
			position.getY() + this->_height / 2.0,
			position.getZ() + this->_depth / 2.0
		)
	);
	return (true);
}
