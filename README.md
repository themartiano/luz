# Luz

**Luz ~ Ray Tracer**

Developed with **C++** using only the *standard library*.

![Luz Spheres](https://user-images.githubusercontent.com/32342284/138613098-84a5b9c0-e675-41a2-ac91-523510d41d7d.jpg)
###### Raw output converted from .bmp to .jpg
---

## Features:
### Rendering:
* **Path Tracing**
* **Physically based** rendering
* **Sphere rendering** with customizable position, radius and material
* **Rectangle rendering** with customizable position, orientation, size and material
* **Plane rendering** with customizable Y position, orientation and material
* Lambertian (**Diffuse**) Rendering
* **Metals** with custom reflection clarity
* Dielectrics (**Glass**, etc)
* Emissive materials (**light** emitting objects)
* **Atmosphere** rendering (physically based)
* **Sky** rendering with customizable skyline (interpolation)
* Custom (solid) background color (if sky and atmosphere rendering is disabled)
* **Anti-aliasing**
* **Depth of Field**
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

## Showcase:
### Cornell Box:
![Luz Cornell Box](https://user-images.githubusercontent.com/32342284/139581650-2f11c939-92e2-421a-aef8-a1de5809ca4c.jpg)
### Atmosphere:
![Luz Atmosphere](https://user-images.githubusercontent.com/32342284/139664916-67791780-419d-4c59-a307-464af07496cb.jpg)
###### Raw output converted from .bmp to .jpg

---

## Compilation:
* Makefile ready for Linux, MacOS and Windows compilation
* Requires **clang++** (Linux & MacOS compilation) **or mingw** (Windows compilation)
### Makefile flags:
* **DEBUG**: Adds the `-g` flag
* **SANITIZER**: Adds the `-g` and `-fsanitize=address` flags
* **NO_FLAGS**: Removes `-Wall`, `-Wextra` and `-Werror` flags
* **COMPILER**: Uses `clang++` (0) or `mingw` (1)

---

## Useful links:
* https://en.wikipedia.org/wiki/Schlick%27s_approximation
* https://en.wikipedia.org/wiki/List_of_refractive_indices
* https://raytracing.github.io/
* https://mikeadev.net/2019/11/parallelizing-ray-tracing/
* https://en.wikipedia.org/wiki/Photon_mapping
* https://en.wikipedia.org/wiki/Spectral_rendering
* https://noahpitts.github.io/godRay/
* https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky
* http://nishitalab.org/user/nis/cdrom/sig93_nis.pdf
* https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html
* https://docs.fileformat.com/image/bmp/
* https://en.wikipedia.org/wiki/BMP_file_format

##### by Brhaka
