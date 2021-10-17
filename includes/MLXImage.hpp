#ifndef MLXIMAGE_HPP
# define MLXIMAGE_HPP

class	MLXImage
{
	public:
		MLXImage(void);
		void	setImg(void* img);
		void*	getImg(void) const;
		void	setAddress(char* address);
		void	setBitsPerPixel(int bits_per_pixel);
		void	setLineLength(int line_length);
		void	setEndian(int endian);

	private:
		void	*_img;
		char	*_address;
		int		_bits_per_pixel;
		int		_line_length;
		int		_endian;
};

#endif