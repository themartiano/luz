#ifndef BVHNODE_HPP
#define BVHNODE_HPP

#include "Hittable.hpp"
#include <vector>
#include <memory>

class   BVHNode : public Hittable
{
    public:
        BVHNode(void);
        virtual ~BVHNode(void) = default;
        BVHNode(std::vector<std::shared_ptr<Hittable>> hittables);
        BVHNode(std::vector<std::shared_ptr<Hittable>> hittables, size_t start, size_t end);
		virtual bool    hit(Ray& ray, float t_max) const override;
		virtual bool    createBoundingBox(AABB& outputBoundingBox) const override;

    private:
        std::shared_ptr<Hittable>   _left;
        std::shared_ptr<Hittable>   _right;
        AABB                        _boundingBox;
};

#endif