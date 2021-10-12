#ifndef MATERIAL_HPP
# define MATERIAL_HPP

class	Material
{
	public:
		Material(void);

	private:
		t_color		_color;
		t_transform	_transform;
		float		_brightness;

};

#endif