# Luz

**Luz ~ Ray Tracer**

Developed with modern **C++ 17** using nothing but the language itself.

![Luz Spheres](https://user-images.githubusercontent.com/32342284/138613098-84a5b9c0-e675-41a2-ac91-523510d41d7d.jpg)
###### Raw output converted from bmp to jpg
---

## Features:
### Rendering:
* **Monte Carlo Path Tracing**
* **Physically based** rendering
* **Multithreaded** rendering
* **Objects** rendering (sphere, plane, rectangle, triangle, cube)
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
* **.bmp** image file creation with the resulting render
* Customizable resolution
### Terminal output:
* Progress indicators
* Render time indicator
* **Colored output**

---

## Showcase:
### Cornell Box
![Luz Cornell Box](https://user-images.githubusercontent.com/32342284/139581650-2f11c939-92e2-421a-aef8-a1de5809ca4c.jpg)
### Atmosphere
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
