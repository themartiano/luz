cmake_minimum_required(VERSION 3.16)

project(luz LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LUZ_NATIVE_OPTIMIZATIONS "Build release binaries with native CPU tuning" ON)
option(LUZ_ENABLE_LTO "Enable link-time optimization for release builds" ON)

if(LUZ_ENABLE_LTO)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT LUZ_IPO_SUPPORTED OUTPUT LUZ_IPO_ERROR)
	if(LUZ_IPO_SUPPORTED)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
	else()
		message(WARNING "LTO was requested but is not supported: ${LUZ_IPO_ERROR}")
	endif()
endif()

function(luz_configure_target target)
	target_include_directories(${target} PUBLIC include/luz)
	target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
	target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:-O3>)

	if(LUZ_NATIVE_OPTIMIZATIONS)
		if(MSVC)
			target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:/arch:AVX2>)
		else()
			target_compile_options(${target} PRIVATE $<$<CONFIG:Release>:-march=native>)
		endif()
	endif()
endfunction()

file(GLOB_RECURSE LUZ_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM LUZ_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.cpp")

add_library(luz_core STATIC ${LUZ_SOURCE_FILES})
luz_configure_target(luz_core)

add_executable(luz src/cli/main.cpp)
target_link_libraries(luz PRIVATE luz_core)
luz_configure_target(luz)

enable_testing()
add_executable(luz_tests tests/Tests.cpp)
target_link_libraries(luz_tests PRIVATE luz_core)
luz_configure_target(luz_tests)
add_test(NAME luz_tests COMMAND luz_tests)
set_tests_properties(luz_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
