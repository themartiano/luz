#pragma once

#include "SmartArray.hpp"
#include "Color.hpp"
#include "ColorManagement.hpp"
#include <memory>

class	Image
{
	public:
		Image(void);
		Image(const Image &image) = default;
		Image(std::size_t width, std::size_t height);
		std::size_t	getWidth(void) const;
		void		setWidth(std::size_t width);
		std::size_t	getHeight(void) const;
		void		setHeight(std::size_t height);
		Color		getPixel(std::size_t x, std::size_t y) const;
		void		setPixel(std::size_t x, std::size_t y, Color color);
		Color		getPixelUnchecked(std::size_t x, std::size_t y) const;
		void		setPixelUnchecked(std::size_t x, std::size_t y, Color color);
		const SmartArray<Color>&	data(void) const;
		Color*		pixels(void);
		const Color*	pixels(void) const;
		void		saveToBMP(const std::string &filename) const;
		void		saveToTIFF(const std::string &filename) const;
		void		saveToPNG(const std::string &filename) const;
		ImageColorEncoding	getColorEncoding(void) const;
		void		setColorEncoding(ImageColorEncoding colorEncoding);
		void		initialize(void);
		Color&		at(std::size_t index);
		void		fill(Color color);
		void		applyExposure(double exposure);
		void		applyContrast(double contrast);
		void		gammaCorrect(void);
		void		applyViewTransform(ViewTransform viewTransform);
		void		applyViewTransformAndEncodeSRGB(ViewTransform viewTransform);
		void		suppressIsolatedFireflies(void);
		std::unique_ptr<Image>	extractBloom(double threshold, double softKnee) const;
		std::unique_ptr<Image>	extractBrightness(void) const;

		Image&	operator+=(const Image& other);
		Image&	operator=(const Image& other);
		Color&	operator[](std::size_t index);

	private:
		SmartArray<Color>	_pixels;
		std::size_t			_width;
		std::size_t			_height;
		ImageColorEncoding	_colorEncoding;
		bool				_initialized;

		void	_checkInitialized(void) const;
};
