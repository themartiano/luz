# Luz

Luz is a hand-written C++20 path tracer built with only the C++ standard library.

![Cornell Box render](docs/images/cornell-box.jpg)

## Features

- Monte Carlo path tracing
- Multithreaded rendering
- Spheres, planes, rectangles, triangles, cubes, volumes, and OBJ meshes
- Lambertian, metal, dielectric, emissive, and isotropic materials
- Custom `.luz` scene files
- OBJ mesh loading
- Importance sampling with PDFs
- BVH acceleration
- Atmospheric scattering
- Depth of field, antialiasing, tone mapping, gamma correction, and bloom
- BMP and TIFF output

## Quick Start

Build with the Makefile:

```sh
make
```

Render the bundled demo scene:

```sh
./Luz --file examples/scenes/demo.luz --samples 8 --resolution 300x300 --threads 4
```

The default output is `render.bmp`. Scene files can set `outputfilename=...`, and the CLI can override common render settings.

Run the test suite:

```sh
make test
```

Run the deterministic benchmark scene:

```sh
./Luz --benchmark --seed 424242424 --threads 1
```

## CMake

A CMake build is also available:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## CLI

```text
Usage: ./Luz [options]

  -f, --file PATH             Load a .luz scene file
  -r, --resolution WxH        Override render resolution
  -s, --samples N             Override samples per pixel
  -mlb, --maxLightBounces N   Override maximum light bounces
  -t, --threads N             Render with N worker threads
  --seed N                    Seed random sampling
  --gamma true|false          Toggle gamma correction
  -tm, --tonemapping true|false  Toggle tone mapping
  --bloom true|false          Toggle bloom
  --render-times              Write renderTime.bmp
  --benchmark                 Run the built-in benchmark scene
```

## Scene Files

Example scenes live in `examples/scenes/`. Mesh assets live in `assets/objects/`.

Object paths in `.luz` files are resolved relative to the scene file first, then relative to the current working directory, then under `assets/objects/`. This means `examples/scenes/demo.luz` can reference `../../assets/objects/pyramid.obj` and still run from the repository root.

## Repository Layout

```text
include/luz/       Public headers
src/core/          Math, geometry, materials, image, and sampling code
src/renderer/      Rendering implementation
src/scene/         Scene model and scene helpers
src/io/            Scene-file, OBJ, BMP, and TIFF loading/writing
src/cli/           Command-line entry point and flags
examples/scenes/   Example .luz scene files
assets/objects/    OBJ assets used by examples
docs/images/       Compressed showcase images
tests/             Standard-library-only test program
docker/            Benchmark container
```

## Showcase

![Glass monkey render](docs/images/glass-monkey.jpg)

![Earth atmosphere from space](docs/images/atmosphere-from-space.jpg)

## License

MIT. See `LICENSE`.
