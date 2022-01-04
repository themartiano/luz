#include "BVHNode.hpp"
#include "Utilities.hpp"
#include "Defaults.hpp"
#include <algorithm>

// Static function prototypes
static bool    boxXCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool    boxYCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool    boxZCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2);
static bool    boxCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2, int axis);

/*
	Constructors
*/

// Constructor overload, only calls the actual constructor
BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>> hittables) : BVHNode(hittables, 0, hittables.size())
{}


// Constructs the BVHNode
BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>> hittables, size_t start, size_t end)
{
    int axis = Utilities::randomInt(0, 2);
    auto comparator = (axis == 0) ? boxXCompare : (axis == 1) ? boxYCompare : boxZCompare;

    int hittableCount = end - start;

    if (hittableCount == 1)
    {
        this->_left = this->_right = hittables[start];
    }
    else if (hittableCount == 2)
    {
        if (comparator(hittables[start], hittables[start + 1]))
        {
            this->_left = hittables[start];
            this->_right = hittables[start + 1];
        }
        else
        {
            this->_left = hittables[start + 1];
            this->_right = hittables[start];
        }
    }
    else
    {
        std::sort(hittables.begin() + start, hittables.begin() + end, comparator);

        auto mid = start + hittableCount / 2;
        this->_left = std::make_shared<BVHNode>(hittables, start, mid);
        this->_right = std::make_shared<BVHNode>(hittables, mid, end);
    }

    AABB boxLeft;
    AABB boxRight;

    if (!this->_left->createBoundingBox(boxLeft) || !this->_right->createBoundingBox(boxRight))
    {
        // Error
    }

    this->_boundingBox = Utilities::mergeBoundingBoxes(boxLeft, boxRight);
}

// Comparator for the BVH box at the X axis
static bool    boxXCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
    return (boxCompare(hittable1, hittable2, 0));
}

// Comparator for the BVH box at the Y axis
static bool    boxYCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
    return (boxCompare(hittable1, hittable2, 1));
}

// Comparator for the BVH box at the Z axis
static bool    boxZCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2)
{
    return (boxCompare(hittable1, hittable2, 2));
}

// Compares 'hittable1' and 'hittable2'
static bool    boxCompare(std::shared_ptr<Hittable> hittable1, std::shared_ptr<Hittable> hittable2, int axis)
{
    AABB    boxA;
    AABB    boxB;

    if (hittable1->createBoundingBox(boxA) && hittable2->createBoundingBox(boxB))
    {
        return (boxA.getMinimum()[axis] < boxB.getMinimum()[axis]);
    }

    // Error
    return (false);
}

// Checks if the BVH Node is hit, then checks if the left or right nodes are hit
bool    BVHNode::hit(Ray& ray, double t_max) const
{
    if (!this->_boundingBox.hit(ray, t_max))
    {
        return (false);
    }

    if (RENDER_AABB)
    {
        ray.hitRecord.material = Material(Color(0.0, 0.0, 0.0), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);

        return (true);
    }
    else
    {
        bool hitLeft = this->_left->hit(ray, t_max);
        bool hitRight = this->_right->hit(ray, hitLeft ? ray.hitRecord.t0 : t_max);

        return (hitLeft || hitRight);
    }
}

// Sets 'outputBoundingBox' to the BVH Node's '_boundingBox'
bool    BVHNode::createBoundingBox(AABB& outputBoundingBox) const
{
    outputBoundingBox = this->_boundingBox;
    return (true);
}

// Returns an empty Material (this function only exists because it must be implemented since the Hittable class has it)
Material    BVHNode::getMaterial(void) const
{
    return (Material());
}
