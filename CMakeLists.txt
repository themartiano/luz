cmake_minimum_required(VERSION 3.16)

project(Luz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE LUZ_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM LUZ_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.cpp")

add_library(luz_core STATIC ${LUZ_SOURCE_FILES})
target_include_directories(luz_core PUBLIC include/luz)
target_compile_options(luz_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(Luz src/cli/main.cpp)
target_link_libraries(Luz PRIVATE luz_core)
target_compile_options(Luz PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_executable(LuzTests tests/Tests.cpp)
target_link_libraries(LuzTests PRIVATE luz_core)
target_compile_options(LuzTests PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME LuzTests COMMAND LuzTests)
set_tests_properties(LuzTests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
