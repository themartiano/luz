#ifndef DEFAULTS_HPP
#define DEFAULTS_HPP

#include <limits>
#include <thread>

const int	D_WIDTH = 1920; // Default screen width in pixels
const int	D_HEIGHT = 1080; // Default screen height in pixels

const int	D_SAMPLE_COUNT = 48; // Default sample count (rays per pixel)
const int	D_MAX_LIGHT_BOUNCES = 12; // Default maximum light bounces per ray

const double T_MAX = std::numeric_limits<double>::max(); // Default T_MAX (as far as an object can be in order to be rendered, relatively to the camera's position)
const double T_MIN = 0.001; // Default T_MIN (as near as an object can be in order to be rendered, relatively to the camera's position)

const double D_EARTH_RADIUS = 6360e3;
const double D_ATMOSPHERE_RADIUS = 6420e3;
const double D_HR = 7994.0;
const double D_HM = 1200.0;

const unsigned int CORE_COUNT = std::thread::hardware_concurrency();

#endif