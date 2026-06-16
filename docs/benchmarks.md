# Benchmarks

Luz includes deterministic benchmark workflows for measuring renderer changes
across render time, denoise time, post-process time, and overall score.

## Quick Comparison

Run the containerized benchmark matrix and save raw results:

```sh
make benchmark BENCH_CPUS=1 BENCH_THREADS=1 > before.csv
```

After an optimization, run the same matrix again and compare medians:

```sh
make benchmark BENCH_CPUS=1 BENCH_THREADS=1 > after.csv
make benchmark-compare BEFORE=before.csv AFTER=after.csv
```

The benchmark score is printed to stderr at the end of the run, so redirecting
stdout still writes a clean raw CSV.

## Direct Runs

Run the deterministic default benchmark case without the container:

```sh
./luz --benchmark --seed 424242424 --threads 1
```

Run a scene in benchmark mode:

```sh
./luz examples/scenes/cornell.luz --resolution 320x180 --samples 128 --max-light-bounces 5 --benchmark
```

## Scoring Results

Score an existing benchmark CSV:

```sh
make benchmark-score RESULTS=after.csv
```

Each case score is the median kilo-samples per minute for that case. The overall
score uses the geometric mean of per-case scores, so `before.csv` and
`after.csv` are comparable when benchmark settings are the same.

## CSV Output

Raw benchmark CSVs include:

- Elapsed time
- Render time
- Denoise time
- Post-process time
- Total samples rendered
- Average samples per pixel
- Actual score
- Render-only score

The compare script reports elapsed speedup, render-time speedup, score speedup,
render-score speedup, and how much non-render work is in each case.

## Benchmark Matrix

The default matrix covers Cornell-style lighting, many objects, mesh BVH
traversal, diffuse scattering, post-processing, atmosphere, mixed light types,
emissive geometry, primitives/materials, volumes, OBJ meshes, and representative
scene cases.

The Makefile currently requests these default cases:

```text
default many-objects mesh-bvh diffuse postprocess atmosphere lights
emissive-geometry primitives-materials volumes obj-mesh
stormtroopers-preview stormtroopers-adaptive-tuned stormtroopers-denoise-micro
```

Scene-backed cases are skipped if their scene file is not present. For example,
the stormtrooper cases use `exports/stormtroopers.luz` when that export exists.

## Useful Settings

Keep benchmark settings fixed between before/after runs. The most common knobs
are:

- `BENCH_CPUS`: CPU quota passed to Docker
- `BENCH_THREADS`: Luz render worker count
- `BENCH_REPEAT`: measured runs per case
- `BENCH_WARMUP`: warmup runs per case
- `BENCH_SEED`: deterministic render seed
- `BENCH_CASES`: space-separated benchmark cases
- `BENCH_WIDTH` and `BENCH_HEIGHT`: override case resolution
- `BENCH_SAMPLES`: override samples per pixel
- `BENCH_DENOISE`: override the default denoising state
- `BENCH_ADAPTIVE`: override the default adaptive sampling state
- `BENCH_SCORE_SAMPLE_UNIT`: score divisor, defaulting to `1000`
