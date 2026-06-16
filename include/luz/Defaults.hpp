#pragma once

#include <limits>
#include <thread>
#include <string>
#include <cstdint>

const std::size_t	D_WIDTH = 1920; // Default screen width in pixels
const std::size_t	D_HEIGHT = 1080; // Default screen height in pixels

const int	D_SAMPLE_COUNT = 48; // Default sample count (rays per pixel)
const int	D_MAX_LIGHT_BOUNCES = 6; // Default maximum light bounces per ray
const bool	D_ADAPTIVE_SAMPLING = true; // Default adaptive sampling state
const int	D_ADAPTIVE_MIN_SAMPLES = 128; // Default minimum samples before adaptive sampling may stop a pixel
const int	D_ADAPTIVE_CHECK_INTERVAL = 32; // Default adaptive sampling convergence check interval
const double D_ADAPTIVE_THRESHOLD = 0.03; // Default relative 95% confidence interval threshold
const bool	D_DENOISE = true; // Default denoising state

const double T_MAX = std::numeric_limits<double>::max(); // Default T_MAX (as far as an object can be in order to be rendered, relatively to the camera's position)
const double T_MIN = 0.001; // Default T_MIN (as near as an object can be in order to be rendered, relatively to the camera's position)

const double D_EARTH_RADIUS = 6360e3;
const double D_ATMOSPHERE_RADIUS = 6420e3;
const double D_HR = 7994.0;
const double D_HM = 1200.0;

const unsigned int CORE_COUNT = std::thread::hardware_concurrency(); // CPU core count

const bool RENDER_AABB = false; // Used to determine if AABBs should be rendered (for debugging purposes).

const double D_PI = 3.14159265358979323846; // Default PI value (same as the one from <cmath>, ATM)

const std::string D_RENDER_FILE_NAME = "render";

const double D_VOLUME_DENSITY = 1.42;

const double D_MAX_RAY_COLOR_LUMINANCE = 10.0;

const double D_EXPOSURE = 0.0; // Default exposure compensation in stops
const double D_CONTRAST = 1.0; // Default display contrast multiplier
const double D_BLOOM_THRESHOLD = 1.0; // Scene-linear luminance threshold for bloom
const double D_BLOOM_SOFT_KNEE = 0.5; // Fractional soft knee around the bloom threshold
const double D_BLOOM_INTENSITY = 0.25; // Amount of blurred bloom added back to the HDR image
