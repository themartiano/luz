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

    // Update faces
    _generateFaces();
}

// Constructs the Cube with custom values
Cube::Cube(Transform transform, double width, double height, double depth, Material material)
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
void    Cube::setTransform(Transform transform)
{
    this->_transform = transform;
}

// Sets the Cube's Material
void    Cube::setMaterial(Material material)
{
    this->_material = material;
}

// Sets the Cube's Width
void    Cube::setWidth(double width)
{
    this->_width = width;
}

// Sets the Cube's Height
void    Cube::setHeight(double height)
{
    this->_height = height;
}

// Sets the Cube's Depth
void    Cube::setDepth(double depth)
{
    this->_depth = depth;
}

// Updates the Cube's faces
void    Cube::_generateFaces(void)
{
    Vector3 position = this->_transform.getPosition();

    // Front
    this->_faces.push_back(Rectangle(Transform(position - Vector3(0.0, 0.0, this->_depth / 2.0), Vector3(0.0, 0.0, -1.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

    // Back
    this->_faces.push_back(Rectangle(Transform(position + Vector3(0.0, 0.0, this->_depth / 2.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

    // Top
    this->_faces.push_back(Rectangle(Transform(position + Vector3(0.0, this->_height / 2.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

    // Bottom
    this->_faces.push_back(Rectangle(Transform(position - Vector3(0.0, this->_height / 2.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

    // Right
    this->_faces.push_back(Rectangle(Transform(position + Vector3(this->_width / 2.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));

    // Left
    this->_faces.push_back(Rectangle(Transform(position - Vector3(this->_width / 2.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0)), this->_width, this->_height, this->_material));
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Cube::hit(Ray& ray, double t_max) const
{
    for (Rectangle rectangle : this->_faces)
    {
        if (rectangle.hit(ray, t_max))
        {
            return (true);
        }
    }

    return (false);
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

