#include "Ray.hpp"

Ray::Ray(void)
{
    this->_origin = Vector3(0.0f, 0.0f, 0.0f);
    this->_direction = Vector3(0.0f, 0.0f, 0.0f);
}

Ray::Ray(Vector3 origin, Vector3 direction, Scene scene, int x, int y)
{
	this->_origin = origin;

	Vector3 view_up(0.0f, 1.0f, 0.0f);
	Vector3 w = normalize(origin - direction);
	Vector3 u = normalize(cross(view_up, w));
	Vector3 v = normalize(cross(w, u));

    float   halfWidth = tan(((float)scene.getActiveCamera().getFOV() * M_PI / 180.0f) / 2.0f);
    float   halfHeight = ((float)scene.getYResolution() / (float)scene.getXResolution()) * halfWidth;

    Vector3 newDir;
	newDir.setX((halfWidth * -1.0f) + direction.getX() + (x * u.getX() * halfWidth * 2.0f));
	newDir.setY((halfHeight * -1.0f) + direction.getY() + (y * v.getY() * halfHeight * 2.0f));
	newDir.setZ(direction.getZ());
	newDir = normalize(newDir);
	newDir.setY(newDir.getY() * -1.0f);

    this->_direction = newDir;
}

// Returns a point on the ray where 't' determines the position
Vector3 Ray::pointAtRay(float t)
{
    return (this->_origin + (Vector3)(this->_direction * t));
}

Vector3 Ray::getOrigin(void) const
{
    return (this->_origin);
}

Vector3 Ray::getDirection(void) const
{
    return (this->_direction);
}
