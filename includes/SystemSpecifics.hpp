#ifndef SYSTEMSPECIFICS_HPP
#define SYSTEMSPECIFICS_HPP

#ifndef OS
#define OS 0
#endif

#if OS == 1
#include "WindowsSpecifics.hpp"
#endif

#endif