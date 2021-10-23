# Luz

## Features:
* Customizable resolution
* .bmp image file creation with the resulting render
* Progress indicators
* Render time indicator
* Colored output
* Functions descriptions
### Rendering features:
* Sphere rendering with customizable position, radius, color and albedo
* Lambertian (Diffuse)
* Metals with custom reflections-fuzziness
* Dielectrics (Glass, etc)
* Anti-aliasing
* Depth of Field
* Sky with customizable skyline
* Light bouncing (reflection & refraction)
* Per-pixel rendering
* Gamma correction (optional)
* Customizable multi-sampled rendering
* Customizable maximum light bounces count
### Camera features:
* Customizable Field of View
* Customizable Position
* Customizable Aperture

---

## Compilation:
* Makefile ready for Linux and MacOS
* Requires clang++
### Flags:
* DEBUG: Adds the `-g` flag
* NO_FLAGS: Removes `-Wall`, `-Wextra` and `-Werror` flags
* SANITIZER: Adds the `-g` and `-fsanitize=address` flags

##### Usage example: make NO_FLAGS=1
