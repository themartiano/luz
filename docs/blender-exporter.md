# Blender Exporter

Luz can render Blender scenes through a generated `.luz` scene file and OBJ
mesh files. The exporter runs inside Blender so it can read `.blend` files
through Blender's own Python API.

## Usage

From the repository root:

```sh
"/Applications/Blender.app/Contents/MacOS/Blender" -b scene.blend --python tools/blender_export_luz.py -- --output exports/scene.luz
```

If your user add-ons make startup noisy or slow, run Blender with factory startup
settings:

```sh
"/Applications/Blender.app/Contents/MacOS/Blender" --factory-startup -b scene.blend --python tools/blender_export_luz.py -- --output exports/scene.luz
```

Then render with Luz:

```sh
./luz --file exports/scene.luz --threads 8
```

The exporter also works without passing the `.blend` file before `--python`:

```sh
"/Applications/Blender.app/Contents/MacOS/Blender" -b --python tools/blender_export_luz.py -- --blend scene.blend --output exports/scene.luz
```

## Options

```text
--output PATH                 Output .luz scene file. Required.
--mesh-dir DIR                OBJ output directory. Defaults to meshes next to the .luz file.
--selected-only               Export selected objects only.
--include-hidden              Include objects hidden from render.
--resolution WIDTHxHEIGHT     Override exported render resolution.
--samples N                   Override samples per pixel.
--adaptive auto|on|off        Enable adaptive sampling by default, mirror Blender with auto, or force off.
--adaptive-min-samples N      Override exported adaptive minimum samples.
--adaptive-threshold N        Override exported adaptive noise threshold.
--adaptive-check-interval N   Override exported adaptive check interval.
--denoise auto|on|off         Enable denoising by default, mirror Blender with auto, or force off.
--tonemapping auto|on|off     Enable tone mapping by default, mirror Blender with auto, or force off.
--exposure EV                 Override exported display exposure in stops.
--max-light-bounces N         Override max light bounces.
--sky linear|none|atmosphere|environment
                             Override exported Luz sky mode.
--render-output PATH          Luz render output path.
--global-scale N              Scale exported positions, meshes, and light sizes.
--light-power-scale N         Extra multiplier for exported area/point power. Defaults to 1.0.
--min-point-light-radius N    Minimum Blender-unit radius for point/spot lights. Defaults to 0.1.
--sun-power-scale N           Multiplier for exported sun irradiance. Defaults to 1.0.
--camera-f-stop N             Override exported camera f-number.
--texture-dir DIR             Texture output directory. Defaults to textures next to the .luz file.
--texture-max-size N          Maximum exported texture width/height. Defaults to 1024.
--no-texture-colors           Skip image texture color approximation.
--texture-sample-size N       Thumbnail size for image color averaging. Defaults to 64.
--profile                     Print per-stage exporter progress and timings.
```

## What Gets Exported

- Mesh objects are evaluated with modifiers, triangulated, split by material
  slot, and written as simple Luz-compatible OBJ files.
- Blender world transforms are baked into OBJ vertices for reliable placement.
- Blender's Z-up world is converted to Luz's Y-up world.
- Evaluated Blender corner normals are exported as OBJ `vn` data so Luz can
  preserve smooth shading.
- Active Blender UV coordinates are exported as OBJ `vt` data and used by Luz
  when a material has a texture.
- Blender World camera background is exported as `sky=none` plus
  `background=(R,G,B)` unless an Environment Texture is found or `--sky`
  overrides the sky mode. Light Path mixes that separate camera rays from
  non-camera rays are handled for the camera background only.
- Blender World Environment Texture nodes connected to Background Color are
  exported as `environment=PATH,ROTATION` plus `environment_scale=STRENGTH`.
  External `.hdr` and `.pic` files are copied through as HDR environment maps;
  generated, packed, or non-HDR environment images are exported as PPM. If
  `--sky atmosphere` is used, the HDRI is still exported and lights the scene
  behind the atmospheric sky.
- The active camera is exported as a Luz camera block.
- Blender camera roll is preserved through the named camera `up` vector.
- The exporter writes `meters_per_unit=1 / --global-scale` so physical light
  power, light area, and atmosphere distances preserve Blender-meter scale even
  when exported coordinates are scaled for Luz.
- Invalid zero camera focus distances are replaced with a scene-based fallback.
- Blender camera lens, sensor size, f-stop, and focus distance are exported as
  physical Luz camera properties. Focus distance remains in meters even when
  `--global-scale` changes exported coordinates; Luz converts it through
  `meters_per_unit` while rendering.
- Luz fits the exported physical sensor gate to the render resolution aspect, so
  rendering a full-frame 36x24 mm camera at 16:9 crops the vertical gate instead
  of stretching the image.
- Blender cameras with DOF disabled are exported as `pinhole=1`. Cameras with
  valid DOF export Blender's `aperture_fstop` as `f_stop`.
- Area lights become `area_light` blocks. Blender's area-light energy is treated
  as total emitted power and written as `power=energy * --light-power-scale`.
  Luz converts power to surface radiance from the exported light area.
- Point and spot lights become small emissive `point_light` sphere blocks.
  Blender's point-light energy is treated as total emitted power and written as
  `power=energy * --light-power-scale`. Luz converts power to sphere surface
  radiance from the exported radius. When Blender's light softness is zero,
  `--min-point-light-radius` prevents near-zero export radii from producing
  extreme surface radiance. Exported point and spot lights use `visible=0` so
  their emissive sampling spheres do not render as visible bulbs.
- Sun lights become `directional_light` blocks. They use
  `irradiance=energy * --sun-power-scale`, not `--light-power-scale`, because
  Blender SUN energy is directional rather than finite-area power.
- When `--sky atmosphere` is used, the first exported Blender SUN light becomes
  the source of truth for both surface lighting and atmosphere lighting,
  including sun direction, color, and irradiance. The exporter still writes an
  `atmosphere=` line for the physical atmosphere parameters; its sun-angle field
  and atmosphere fallback radiance are only fallbacks when no `directional_light`
  exists in the scene.
- Emissive mesh materials stay as emissive mesh objects and write
  `radiance=...`. Luz importance-samples emissive meshes directly.
- Common Blender shader nodes connected to Material Output are mapped into
  Luz material properties, including diffuse, glossy, glass, emission,
  Principled, mix, add, RGB/value, image average color, and color ramp values.
- Image textures connected directly to Principled BSDF Base Color or Diffuse
  BSDF Color are exported as PPM textures, downscaled by `--texture-max-size`,
  and referenced from the generated named material with `texture=...`. Luz loads
  these base-color PPMs as sRGB albedo textures and converts them into its
  scene-linear ACEScg working space.
- When a material has multiple Material Output nodes, the exporter prefers the
  output target that matches the scene render engine, such as Cycles or EEVEE.
- Blender material values are written as named material property blocks for
  Luz's scene parser to approximate.
- Blender RGB material, light, and world colors are written as
  `linear_srgb(...)` values so Luz converts Blender's linear RGB values into its
  ACEScg working space.
- Adaptive sampling, denoising, and tone mapping are enabled in the generated
  scene by default. Pass `--adaptive auto`, `--denoise auto`, or
  `--tonemapping auto` to mirror Blender's corresponding setting, or pass `off`
  to disable a feature explicitly.
- When Blender leaves adaptive minimum samples unset, the exporter uses a
  conservative minimum derived from the exported sample count instead of relying
  on Luz's generic defaults.
- Blender view exposure is exported as Luz `exposure`. With
  `--tonemapping auto`, Blender `Standard` and `Raw` view transforms export
  `tonemapping=0` so Luz uses direct scene-linear to sRGB display encoding
  instead of its ACES-fitted tone map.
- Principled BSDF export includes IOR, coat weight/roughness, and sheen when
  those Blender sockets are available. Glossy and anisotropic BSDF nodes export
  as Luz `glossy` materials, and Blender Diffuse+Glossy `Mix Shader` graphs
  export as `diffuse_glossy` so legacy Cycles glossy plastic keeps its strong
  reflection without scaling scene lights.

## Current Limits

- Only direct image textures feeding the selected shader's base color are
  exported as texture files. Procedural textures, normal maps, bump maps,
  displacement, and full shader graphs are not exported as texture networks.
- World environment mapping nodes are not exported yet. Environment maps are
  written with Luz's default equirectangular orientation and `0` degrees
  rotation.
- Image texture export uses Luz's built-in PPM texture loader for sRGB base
  color textures. When no usable Base Color image texture is found, image colors
  can still be approximated by averaging a temporary thumbnail, controlled by
  `--texture-sample-size`.
- Per-face materials are preserved by splitting mesh objects into one OBJ per
  material slot.
- Instancing is baked into separate OBJ files.
- Spot lights are approximations, not physically equivalent Blender lights.
- Animations are not exported.
