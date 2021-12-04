#include "Forms/Cube.hpp"
#include "Vector3.hpp"

/*
	Constructors
*/

// Constructs the Cube with default values (width, height, depth = 1.0)
Cube::Cube(void)
{
    this->_transform = Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0));
	this->_material = Material(Color(0.49, 0.49, 0.49), 1.0, 0.0, 0.5, 0.0, false, false, 0.0);
    this->_width = 1.0;
    this->_height = 1.0;
    this->_depth = 1.0;

    _generateFaces();
}

Cube::Cube(Transform transform, Material material, double width, double height, double depth)
{
    this->_transform = transform;
	this->_material = material;
    this->_width = width;
    this->_height = height;
    this->_depth = depth;

    _generateFaces();
}

void    Cube::_generateFaces(void)
{
    Vector3 position = this->_transform.getPosition();

    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
    this->_faces.push_back(Rectangle(Transform(position, Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_material, this->_width, this->_height));
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Cube::hit(Ray& ray, double t_max) const
{

}

// Creates an AABB / bounding box for this Cube
bool    Cube::createBoundingBox(AABB& outputBoundingBox) const
{
	// outputBoundingBox = AABB(
    //     Vector3(this->_x0, this->_y0, this->_position.getZ() - T_MIN),
    //     Vector3(this->_x1, this->_y1, this->_position.getZ() + T_MIN));

    return (true);
    (void)outputBoundingBox;
}

