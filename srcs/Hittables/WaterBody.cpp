#include "Hittables/WaterBody.hpp"
#include "Materials/Dielectric.hpp"
#include "RefractiveIndexes.hpp"

/*
	Constructors
*/

// Constructs the WaterBody with default values
WaterBody::WaterBody(void)
{
	this->_position = Vector3(0.0, 0.0, 0.0);
	this->_size = 20.0;
	this->_material = std::make_shared<Dielectric>(Color(0.027, 0.1254, 0.2), RI_WATER);
	this->_samplesPerSizeUnit = 1.0;
	this->_noiseScale = 1.0;
	this->_magnitude = 10.0;
	this->_depth = 42.0;
	this->_seed = 42;

	this->_perlin = Perlin(this->_seed);
}

// Constructs the WaterBody with custom values
WaterBody::WaterBody(Vector3 position, double size, Color color, double samplesPerSizeUnit, double noiseScale, double magnitude, double depth, unsigned int seed)
{
	this->_position = position;
	this->_size = size;
	this->_material = std::make_shared<Dielectric>(color, RI_WATER);
	this->_samplesPerSizeUnit = samplesPerSizeUnit;
	this->_noiseScale = noiseScale;
	this->_magnitude = magnitude;
	this->_depth = depth;
	this->_seed = seed;

	this->_perlin = Perlin(this->_seed);
}
