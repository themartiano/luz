# Luz

**Luz ~ Ray Tracer**

Developed with modern **C++ 20** using nothing but the language itself.

![Cornell Box 2048 Samples](https://user-images.githubusercontent.com/32342284/176393730-df3e0ab6-f514-4ce0-bfce-80a8014f3279.jpg)
###### Raw output converted from bmp to jpg
---

## Features:
### Rendering:
* **Monte Carlo Path Tracing**
* **Physically based** rendering
* **Multithreaded** rendering
* **Objects** rendering (sphere, plane, rectangle, triangle, cube, mesh)
* Custom **Scene file** (**.luz** extension)
* **OBJ file** rendering (**.obj** extension)
* Lambertian (**Diffuse**) Rendering
* **Metals**
* Dielectrics (**Glass**, etc)
* Emissive materials (**light** emitting objects)
* **Atmospheric** scattering
* **Perlin noise** generation
* Procedural landscape generation
* Procedural water bodies generation
* **Sky** rendering with customizable skyline (linear interpolated)
* Custom (solid) background color (if sky and atmosphere rendering is disabled)
* **Anti-aliasing**
* **Depth of Field**
* Sequence rendering (multiple frames)
* Light bouncing (reflection & refraction)
* Gamma (2) correction
* **Multi-sampled** rendering
* Bounding Volumes
* **Per-pixel** rendering
* Customizable rendering parameters
### Camera:
* Customizable **Field of View**
* Customizable **Position**
* Customizable **Aperture**
### General:
* **.bmp** image file generation
* **.tiff** image file generation
* Customizable resolution
### Terminal output:
* Progress indicators
* Render time indicator
* **Colored output**

---

## Showcase:
#### Mirror, emissive surface and glass
![Mirror, emissive surface and glass](https://user-images.githubusercontent.com/32342284/176394366-067a6d01-1276-4364-a589-8021ce3914a0.jpg)
### Atmosphere
#### Space view (real dimensions)
![Earth's atmosphere viewed from space in real dimensions](https://user-images.githubusercontent.com/32342284/176394736-281915e0-d88b-4166-a4f9-e279c2305583.jpg)
#### Sunset gif
![Luz Atmosphere](https://user-images.githubusercontent.com/32342284/144286415-7fbcf17a-6e2a-454e-8f7c-641998d98e4e.gif)
###### Raw output converted from bmp to jpg / gif

---

## Compilation
* Makefile ready for Linux, MacOS and Windows compilation
* Requires **clang++** (Linux & MacOS compilation) and / or **mingw** (Windows compilation)
### Makefile flags:
* **DEBUG**: Adds the `-g` flag
* **SANITIZER**: Adds the `-g` and `-fsanitize=address` flags
* **NO_FLAGS**: Removes `-Wall`, `-Wextra` and `-Werror` flags
* **COMPILER**: Uses `clang++` (0) or `mingw` (1)

##### by Brhaka
