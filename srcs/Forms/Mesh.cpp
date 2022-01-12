#include "Forms/Mesh.hpp"
#include "Utilities.hpp"

/*
	Constructors
*/

// Constructs the Mesh with default values
Mesh::Mesh(void)
{
	this->_position = Vector3();
	this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
	this->_bvh = BVHNode();
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, Material material, BVHNode bvh)
{
	this->_position = position;
	this->_material = material;
	this->_bvh = bvh;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, Material material, std::vector<std::shared_ptr<Hittable>> triangles)
{
	this->_position = position;
	this->_material = material;
	this->_bvh = BVHNode(triangles);
}

// Returns the Mesh's material
Material	Mesh::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Mesh's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Mesh::hit(Ray& ray, double t_max) const
{
	return (this->_bvh.hit(ray, t_max));
}

// Returns the AABB / bounding box for this Mesh's BVH
bool	Mesh::createBoundingBox(AABB& outputBoundingBox) const
{
	this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
}
