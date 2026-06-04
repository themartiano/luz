#include "Hittables/BVHNode.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include "Materials/Lambertian.hpp"
#include "Random.hpp"
#include <algorithm>

// Static function prototypes
static bool	boxXCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool	boxYCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool	boxZCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool	boxCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2, int axis);

/*
	Constructors
*/

// Constructor overload, only calls the actual constructor
BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>> hittables) : BVHNode(hittables, 0, hittables.size())
{}

// Constructs the BVHNode
BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>> hittables, size_t start, size_t end)
{
	unsigned int axis = randomEngine.integer(0, 2);
	auto comparator = (axis == 0) ? boxXCompare : (axis == 1) ? boxYCompare : boxZCompare;

	size_t	hittableCount = end - start;
	this->_childs.reserve(hittableCount);

	size_t	amount = 15; // Child count
	if (hittableCount <= amount)
	{
		for (size_t i = start; i < end; i++)
		{
			this->_childs.push_back(hittables[i]);
		}
	}
	else
	{
		std::sort(hittables.begin() + start, hittables.begin() + end, comparator);

		auto mid = start + hittableCount / 2;
		this->_childs.push_back(std::make_shared<BVHNode>(hittables, start, mid));
		this->_childs.push_back(std::make_shared<BVHNode>(hittables, mid, end));
	}

	std::vector<AABB>	aabbs;

	for (auto& child : this->_childs)
	{
		AABB	childBoundingBox;
		if (!child->createBoundingBox(childBoundingBox))
		{
			//std::cerr << "Error: BVHNode::BVHNode() - createBoundingBox() failed" << std::endl;
			return;
		}
		aabbs.push_back(childBoundingBox);
	}

	this->_boundingBox = Utilities::mergeBoundingBoxes(aabbs);
}

// Comparator for the BVH box at the X axis
static bool	boxXCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
	return (boxCompare(hittable1, hittable2, 0));
}

// Comparator for the BVH box at the Y axis
static bool	boxYCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
	return (boxCompare(hittable1, hittable2, 1));
}

// Comparator for the BVH box at the Z axis
static bool	boxZCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
	return (boxCompare(hittable1, hittable2, 2));
}

// Compares 'hittable1' and 'hittable2'
static bool	boxCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2, int axis)
{
	AABB	boxA;
	AABB	boxB;

	if (hittable1->createBoundingBox(boxA) && hittable2->createBoundingBox(boxB))
	{
		return (boxA.getMinimum()[axis] < boxB.getMinimum()[axis]);
	}

	// Error
	return (false);
}

// Checks if the BVH Node is hit, then checks if the left or right nodes are hit
bool	BVHNode::hit(Ray& ray, HitRecord& hitRecord, double t_min, double t_max) const
{
	if (!this->_boundingBox.hit(ray, hitRecord, t_max))
	{
		return (false);
	}

	if (RENDER_AABB)
	{
		hitRecord.material = std::make_shared<Lambertian>();

		return (true);
	}
	else
	{
		bool hitAnything = false;
		double closestHit = t_max;

		for (auto& child : this->_childs)
		{
			HitRecord childHitRecord;
			if (child->hit(ray, childHitRecord, t_min, closestHit))
			{
				hitRecord = childHitRecord;
				closestHit = childHitRecord.t0;
				hitAnything = true;
			}
		}

		return (hitAnything);
	}
}

// Sets 'outputBoundingBox' to the BVH Node's '_boundingBox'
bool	BVHNode::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = this->_boundingBox;
	return (true);
}

// Returns an empty Material (this function only exists because it must be implemented since the Hittable class has it)
std::shared_ptr<Material>	BVHNode::getMaterial(void) const
{
	return (std::make_shared<Lambertian>());
}
