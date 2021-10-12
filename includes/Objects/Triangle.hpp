#ifndef TRIANGLE_HPP
# define TRIANGLE_HPP

#include "Vector3.hpp"
#include "Material.hpp"

class	Triangle
{
	public:
		Triangle(void);
		Triangle(Vector3 pos1, Vector3 pos2, Vector3 pos3, Material material);

	private:
		Vector3		_position1;
		Vector3		_position2;
		Vector3		_position3;
		Material	_material;

};

#endif