#pragma once

// 0 => Linux & MacOS, 1 => Windows x86 and x64
#ifndef OS
#define OS 0
#endif

#if OS == 1
#include "WindowsSpecifics.hpp"
#endif
