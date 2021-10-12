#ifndef MLXIMAGE_HPP
# define MLXIMAGE_HPP

class	MLXImage
{
	public:
		MLXImage(void);

	private:
		void	*_img;
		char	*_address;
		int		_bits_per_pixel;
		int		_line_length;
		int		_endian;
};

#endif