#include "OBJReader.hpp"
#include "Hittable.hpp"
#include "BVHNode.hpp"
#include "Forms/Triangle.hpp"
#include <memory>

void    readObj(Scene& scene, std::string fileName)
{
    std::vector<std::shared_ptr<Hittable>> triangles;

    triangles.push_back(std::make_shared<Triangle>(
		Vector3(0.0, 1.0, 0.0),
		Vector3(-1.0, 0.0, 0.0),
		Vector3(1.0, 0.0, 0.0),
		Material(Color(0.8, 1.0, 0.6), 1.0, 0.0, 0.5, 0.0, false, false, 0.0)
	));

    scene.addHittable(std::make_shared<BVHNode>(triangles));
}
