#ifndef DEFAULTS_HPP
# define DEFAULTS_HPP

#include <limits>

const int	D_WIDTH = 1920;
const int	D_HEIGHT = 1080;

const int	D_SAMPLE_COUNT = 48;
const int	D_MAX_LIGHT_BOUNCES = 12;

const float T_MAX = std::numeric_limits<float>::max();
const float T_MIN = 0.0001f;

#endif