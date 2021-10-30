# Luz

**Luz ~ Ray Tracer**

Developed with **C++** using only the *standard library*.

![Luz](https://user-images.githubusercontent.com/32342284/138613098-84a5b9c0-e675-41a2-ac91-523510d41d7d.jpg)
###### Raw output converted from .bmp to .jpg
---

## Features:
### Rendering:
* **Path Tracing**
* **Sphere rendering** with customizable position, radius and material
* **Rectangle rendering** with customizable position, orientation, size and material
* **Plane rendering** with customizable Y position, orientation and material
* Lambertian (**Diffuse**) Rendering
* **Metals** with custom reflection clarity
* Dielectrics (**Glass**, etc)
* Emissive materials (**light** emitting objects)
* **Anti-aliasing**
* **Depth of Field**
* **Sky** with customizable skyline
* Custom (solid) background color (if sky rendering is disabled)
* Light bouncing (reflection & refraction)
* Gamma (2) correction
* **Multi-sampled** rendering
* Bounding Volumes
* **Per-pixel** rendering
* Customizable maximum light bounces count
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

## Compilation:
* Makefile ready for Linux, MacOS and Windows compilation
* Requires **clang++** (Linux & MacOS compilation) **or mingw** (Windows compilation)
### Makefile flags:
* **DEBUG**: Adds the `-g` flag
* **SANITIZER**: Adds the `-g` and `-fsanitize=address` flags
* **NO_FLAGS**: Removes `-Wall`, `-Wextra` and `-Werror` flags
* **COMPILER**: Uses `clang++` (0) or `mingw` (1)

##### by Brhaka
