#ifndef DEFAULTS_HPP
# define DEFAULTS_HPP

#include <limits>

const int	D_WIDTH = 1920; // Default screen width in pixels
const int	D_HEIGHT = 1080; // Default screen height in pixels

const int	D_SAMPLE_COUNT = 4; // Default sample count (rays per pixel)
const int	D_MAX_LIGHT_BOUNCES = 12; // Default maximum light bounces per ray

const float T_MAX = std::numeric_limits<float>::max(); // Default T_MAX (as far as an object can be in order to be rendered, relatively to the camera's position)
const float T_MIN = 0.001f; // Default T_MIN (as near as an object can be in order to be rendered, relatively to the camera's position)

#endif