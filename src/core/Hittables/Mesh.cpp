#include "Hittables/Mesh.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"

/*
	Constructors
*/

// Constructs the Mesh with default values
Mesh::Mesh(void)
{
	this->_position = Vector3();
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_bvh = BVHNode();
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, BVHNode bvh)
{
	this->_position = position;
	this->_material = material;
	this->_bvh = bvh;
}

// Constructs the Mesh with custom values
Mesh::Mesh(Vector3 position, std::shared_ptr<Material> material, std::vector<std::shared_ptr<Hittable>> triangles)
{
	this->_position = position;
	this->_material = material;
	this->_bvh = BVHNode(triangles);
}

// Returns the Mesh's material
std::shared_ptr<Material>	Mesh::getMaterial(void) const
{
	return (this->_material);
}

// Calculates if the Mesh's BVH is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Mesh::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	return (this->_bvh.hit(ray, hitRecord, t_min, t_max));
}

// Returns the AABB / bounding box for this Mesh's BVH
bool	Mesh::createBoundingBox(AABB& outputBoundingBox) const
{
	this->_bvh.createBoundingBox(outputBoundingBox);

	return (true);
}
