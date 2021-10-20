#ifndef SCENE_HPP
# define SCENE_HPP

#include "Camera.hpp"
#include "Light.hpp"
#include "Forms/Triangle.hpp"
#include "Forms/Square.hpp"
#include "Forms/Sphere.hpp"
#include "Forms/Cylinder.hpp"
#include "Forms/Plane.hpp"
#include <vector>

class	Scene
{
	public:
		Scene(void);
		void	addCamera(Camera camera);
		void	addLight(Light light);
		void	addSphere(Sphere sphere);
		int		getXResolution(void) const;
		void	setXResolution(const int xRes);
		int		getYResolution(void) const;
		void	setYResolution(const int yRes);
		int		getSampleCount(void) const;
		void	setSampleCount(const int sampleCount);
		int		getMaxLightBounces(void) const;
		void	setMaxLightBounces(const int maxLightBounces);
		void	setPixelArray(int index, Color pixelColor);
		unsigned char*	getPixelArray() const;
		std::vector<Sphere>	getSpheres(void) const;
		Camera	getActiveCamera(void) const;

	private:
		int						_xResolution;
		int						_yResolution;
		float					_t_max;
		int						_sampleCount;
		int						_maxLightBounces;
		unsigned char*			_pixelArray;
		//Light					_ambientLight;
		std::vector<Camera>		_cameras;
		size_t					_activeCamera;
		std::vector<Light>		_lights;
		std::vector<Triangle>	_triangles;
		std::vector<Square>		_squares;
		std::vector<Sphere>		_spheres;
		std::vector<Cylinder>	_cylinders;
		std::vector<Plane>		_planes;
};

#endif