#include "Hittables/Landscape.hpp"
#include "Materials/Lambertian.hpp"


/*
	Constructors
*/

// Constructs the Landscape with default values
Landscape::Landscape(void)
{
	this->_position = Vector3(0.0, 0.0, 0.0);
	this->_size = 20.0;
	this->_material = std::make_shared<Lambertian>(Color(0.3, 0.29, 0.11));
	this->_samplesPerSizeUnit = 10;
	this->_noiseScale = 1.0;
	this->_magnitude = 10.0;
	this->_depth = 0.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the Landscape with custom values
Landscape::Landscape(Vector3 position, double size, Color color, double samplesPerSizeUnit, double noiseScale, double magnitude, double depth, unsigned int seed)
{
	this->_position = position;
	this->_size = size;
	this->_material = std::make_shared<Lambertian>(color);
	this->_samplesPerSizeUnit = samplesPerSizeUnit;
	this->_noiseScale = noiseScale;
	this->_magnitude = magnitude;
	this->_depth = depth;
	this->_seed = seed;

	this->_perlin = Perlin(this->_seed);
}
