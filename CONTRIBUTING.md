# Contributing to Luz

Thanks for taking the time to improve Luz. This project is a hand-written C++20
path tracer with no third-party runtime dependencies, so contributions should
keep the codebase straightforward, portable, and measurable.

## Getting Started

From the repository root, build the renderer with:

```sh
make
```

Render the bundled demo scene:

```sh
./Luz --file examples/scenes/demo.luz --samples 8 --resolution 300x300 --threads 4
```

Run the test suite:

```sh
make test
```

A CMake build is also available:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Development Guidelines

- Keep changes focused. Prefer small commits that each explain one behavior
  change, bug fix, or documentation update.
- Use C++20 and the standard library. Do not add third-party dependencies
  unless the tradeoff is discussed first.
- Preserve deterministic behavior where possible. Rendering, tests, and
  benchmarks should accept explicit seeds when randomness matters.
- Keep public headers in `include/luz/` and implementation code under the
  matching area of `src/`.
- Add or update tests in `tests/Tests.cpp` for parser behavior, math,
  geometry, image I/O, renderer options, and regressions.
- Update `README.md`, `docs/scene-files.md`, or `docs/blender-exporter.md`
  when a change affects user-facing behavior or file formats.

## Code Style

There is no project formatter checked in. Match the surrounding style:

- Tabs are used for indentation in the existing C++ sources.
- Braces are placed on their own lines for functions, classes, namespaces, and
  control blocks.
- Prefer clear names over comments. Add comments only when they explain a
  non-obvious algorithm, performance constraint, or file-format detail.
- Keep CLI flags, scene-file keywords, and documented examples consistent
  across code, tests, and docs.

## Testing

Before opening a pull request, run:

```sh
make test
```

For memory-safety work, parser changes, or low-level geometry changes, also run:

```sh
make sanitizer
./Luz --file examples/scenes/demo.luz --samples 8 --resolution 300x300 --threads 4
```

If you use CMake locally, verify both build and tests:

```sh
cmake --build build
ctest --test-dir build
```

## Rendering Checks

For rendering or image-output changes, include at least one small render command
in your verification notes. Keep sample counts and resolutions low enough for
reviewers to reproduce quickly, for example:

```sh
./Luz --file examples/scenes/cornell.luz --samples 16 --resolution 320x320 --threads 4 --seed 424242424
```

Use `tools/bmp_metrics.py` when comparing BMP output against an expected image:

```sh
python3 tools/bmp_metrics.py before.bmp after.bmp
```

## Performance Work

Performance changes should include benchmark evidence. The deterministic
containerized benchmark can be run with:

```sh
make benchmark BENCH_CPUS=1 BENCH_THREADS=1 > after.csv
```

To compare against an earlier result:

```sh
make benchmark-compare BEFORE=before.csv AFTER=after.csv
make benchmark-score RESULTS=after.csv
```

Mention the CPU, thread count, benchmark settings, and whether the working tree
was clean. Avoid benchmark-only changes that reduce correctness, determinism, or
scene coverage.

## Scene Files and Assets

- Put small hand-authored examples in `examples/scenes/`.
- Keep reusable OBJ assets in `assets/objects/`.
- Large exported scenes and generated renders should not be added unless they
  are needed for documentation, tests, or reproducible benchmarks.
- When changing the `.luz` format, update `docs/scene-files.md` and add parser
  coverage in `tests/Tests.cpp`.
- When changing Blender export behavior, update `tools/blender_export_luz.py`
  and `docs/blender-exporter.md` together.

## Pull Request Checklist

Before submitting, make sure the pull request includes:

- A concise description of the problem and solution.
- The commands you ran, including `make test` or why it was skipped.
- Screenshots or rendered BMP comparisons for visual changes.
- Benchmark output for performance-sensitive changes.
- Documentation updates for any CLI, scene-file, exporter, or workflow change.

