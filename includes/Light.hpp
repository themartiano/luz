#ifndef LIGHT_HPP
# define LIGHT_HPP

class	Light
{
	public:
		Light(void);

	private:
		t_color		_color;
		t_transform	_transform;
		float		_brightness;

};

#endif