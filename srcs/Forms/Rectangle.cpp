#include "Forms/Rectangle.hpp"
#include "Defaults.hpp"

/*
	Constructors
*/

// Constructs the Rectangle with default values
Rectangle::Rectangle(void)
{
    this->_position = Vector3(0.0f, 0.0f, 0.0f);
    this->_sideSize = 0.0f;
	this->_material = Material(Color(0.49f, 0.49f, 0.49f), 1.0f, 0.0f, 0.5f, 0.0f, false, false, 0.0f);
    this->_x0 = 0.0f;
    this->_x1 = 0.0f;
    this->_y0 = 0.0f;
    this->_y1 = 0.0f;
}

// Constructs the Rectangle with custom values
Rectangle::Rectangle(Vector3 position, float sideSize, Material material)
{
    this->_position = position;
    this->_sideSize = sideSize;
    this->_material = material;
    _calculateCoordinates();
}

// Returns the Rectangle's position (Vector3)
Vector3 Rectangle::getPosition(void) const
{
    return (this->_position);
}

// Returns the Rectangle's side size
float Rectangle::getSideSize(void) const
{
    return (this->_sideSize);
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool    Rectangle::hit(Ray& ray, float t_max) const
{
    float z = this->getPosition().getZ();

    float t = (z - ray.getOrigin().getZ()) / ray.getDirection().getZ();
    if (t < T_MIN || t > t_max)
    {
        return (false);
    }

    float x = ray.getOrigin().getX() + ray.getDirection().getX() * t;
    float y = ray.getOrigin().getY() + ray.getDirection().getY() * t;
    if (x < this->_x0 || x > this->_x1 || y < this->_y0 || y > this->_y1)
    {
        return (false);
    }

    ray.hitRecord.t = t;
    Vector3 outwardsNormal = Vector3(0.0f, 0.0f, 1.0f);
    ray.hitRecord.normal = outwardsNormal;
    ray.hitRecord.material = this->_material;
    ray.hitRecord.position = ray.pointAtRay(t);

    return (true);
}

// Creates an AABB / bounding box for this Rectangle
bool    Rectangle::createBoundingBox(AABB& outputBoundingBox) const
{
	outputBoundingBox = AABB(
        Vector3(this->_x0, this->_y0, this->_position.getZ() - T_MIN),
        Vector3(this->_x1, this->_y1, this->_position.getZ() + T_MIN));

    return (true);
}

// Calculates x0, x1, y0 and y1 using Position and SideSize
void    Rectangle::_calculateCoordinates(void)
{
    float   halfSideSize = this->_sideSize / 2.0f;

    this->_x0 = this->_position.getX() - halfSideSize;
    this->_x1 = this->_position.getX() + halfSideSize;
    this->_y0 = this->_position.getY() - halfSideSize;
    this->_y1 = this->_position.getY() + halfSideSize;
}
