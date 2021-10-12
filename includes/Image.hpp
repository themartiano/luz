#ifndef IMAGE_HPP
# define IMAGE_HPP

class	Image
{
	public:
		Image(void);

	private:
		void	*_img;
		char	*_addr;
		int		_bits_per_pixel;
		int		_line_length;
		int		_endian;

};

#endif