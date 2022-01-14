#include "Forms/Rectangle.hpp"
#include "Defaults.hpp"
#include "Utilities.hpp"
#include "Materials/Lambertian.hpp"
#include <cmath>

/*
	Constructors
*/

// Constructs the Rectangle with default values
Rectangle::Rectangle(void)
{
	this->_transform = Transform(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, 1.0), Vector3(1.0, 1.0, 1.0));
	this->_material = std::make_shared<Lambertian>(Color(0.6, 0.6, 0.6));
	this->_width = 1.0;
	this->_height = 1.0;
}

Rectangle::Rectangle(const Rectangle& toCopy)
{
	this->_height = toCopy._height;
	this->_width = toCopy._width;
	this->_material = toCopy._material;
	this->_transform = toCopy._transform;
}

// Constructs the Rectangle with custom values
Rectangle::Rectangle(Transform transform, double width, double height, std::shared_ptr<Material> material)
{
	this->_transform = transform;
	this->_width = width;
	this->_height = height;
	this->_material = material;
}

// Sets the Rectangle's Transform
void	Rectangle::setTransform(Transform transform)
{
	this->_transform = transform;
}

// Returns the Rectangle's material
std::shared_ptr<Material>	Rectangle::getMaterial(void) const
{
	return (this->_material);
}

// Sets the Rectangle's Material
void	Rectangle::setMaterial(std::shared_ptr<Material> material)
{
	this->_material = material;
}

// Sets the Rectangle's Width
void	Rectangle::setWidth(double width)
{
	this->_width = width;
}

// Sets the Rectangle's Height
void	Rectangle::setHeight(double height)
{
	this->_height = height;
}

// Calculates if the Rectangle is hit by 'ray', is closer than 't_max' and farther than T_MIN
bool	Rectangle::hit(Ray& ray, double t_max) const
{
	double a = Utilities::dot(ray.getOrigin() - this->_transform.getPosition(), this->_transform.getOrientation());
	double b = Utilities::dot(ray.getDirection(), this->_transform.getOrientation());
	if (b == 0.0 || (a < 0.0 && b < 0.0) || (a > 0.0 && b > 0.0))
	{
		return (false);
	}

	double t = -a / b;
	if (t > t_max || t < T_MIN)
	{
		return (false);
	}

	Vector3 d = ray.pointAtRay(t) - this->_transform.getPosition();
	if (fabs(this->_transform.getOrientation().getY()) > 0.0)
	{
		if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getZ()) > (this->_height / 2.0))
		{
			return (false);
		}
	}
	else if (fabs(this->_transform.getOrientation().getZ()) > 0.0)
	{
		if (fabs(d.getX()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_height / 2.0))
		{
			return (false);
		}
	}
	else if (fabs(this->_transform.getOrientation().getX()) > 0.0)
	{
		if (fabs(d.getZ()) > (this->_width / 2.0) || fabs(d.getY()) > (this->_height / 2.0))
		{
			return (false);
		}
	}

	ray.hitRecord.t0 = t;
	ray.hitRecord.normal = this->_transform.getOrientation();
	ray.hitRecord.material = this->_material;
	ray.hitRecord.position = ray.pointAtRay(t);

	return (true);
}

// Creates an AABB / bounding box for this Rectanglstd::shared_ptr<Material>e
bool	Rectangle::createBoundingBox(AABB& outputBoundingBox) const
{
	// outputBoundingBox = AABB(
	//	 Vector3(this->_x0, this->_y0, this->_position.getZ() - T_MIN),
	//	 Vector3(this->_x1, this->_y1, this->_position.getZ() + T_MIN));

	return (true);
	(void)outputBoundingBox;
}

double  Rectangle::pdfValue(const Vector3& origin, const Vector3& vec) const
{
	Ray ray(origin, vec);
	if (!this->hit(ray, T_MAX))
	{
		return (0.0);
	}

	double area = this->_height * this->_width;
	double distanceSquared = ray.hitRecord.t0 * ray.hitRecord.t0 * Utilities::vectorLengthSquared(vec);
	double cosine = fabs(Utilities::dot(vec, ray.hitRecord.normal) / Utilities::vectorLength(vec));

	return (distanceSquared / (cosine * area));
}

// Returns a random point inside the rectangle's area
Vector3 Rectangle::random(const Vector3& origin) const
{
	Vector3 randomPointInsideRectangle(0.0, 0.0, 0.0);

	double x = this->_transform.getPosition().getX();
	double y = this->_transform.getPosition().getY();
	double z = this->_transform.getPosition().getZ();

	double halfWidth = this->_width / 2.0;
	double halfHeight = this->_height / 2.0;

	randomPointInsideRectangle = Vector3(Utilities::randomDouble(x - halfWidth, x + halfWidth), y, Utilities::randomDouble(z - halfHeight, z + halfHeight));

	// if (fabs(this->_transform.getOrientation().getY()) > 0.0)
	// {
	//	 randomPointInsideRectangle = Vector3(Utilities::randomDouble(x - halfWidth, x + halfWidth), y, Utilities::randomDouble(z - halfHeight, z + halfHeight));
	// }
	// else if (fabs(this->_transform.getOrientation().getZ()) > 0.0)
	// {
	//	 randomPointInsideRectangle = Vector3(Utilities::randomDouble(x - halfWidth, x + halfWidth), z, Utilities::randomDouble(y - halfHeight, y + halfHeight));
	// }
	// else if (fabs(this->_transform.getOrientation().getX()) > 0.0)
	// {
	//	 randomPointInsideRectangle = Vector3(Utilities::randomDouble(z - halfWidth, z + halfWidth), x, Utilities::randomDouble(y - halfHeight, y + halfHeight));
	// }

	return (Utilities::normalize(randomPointInsideRectangle - origin));
}
