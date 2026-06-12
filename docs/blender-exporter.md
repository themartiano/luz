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
./Luz --file exports/scene.luz --threads 8
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
--max-light-bounces N         Override max light bounces.
--sky linear|none|atmosphere  Override exported Luz sky mode.
--render-output PATH          Luz render output path.
--global-scale N              Scale exported positions, meshes, and light sizes.
--light-power-scale N         Extra multiplier after Blender lamp energy conversion. Defaults to 1.0.
--min-point-light-radius N    Minimum Blender-unit radius for point/spot lights. Defaults to 0.1.
--sun-power-scale N           Convert Blender sun energy to Luz intensity. Defaults to 1.0.
--camera-aperture N           Override Luz camera aperture.
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
  `background=(R,G,B)` unless `--sky` overrides the sky mode. Light Path mixes
  that separate camera rays from non-camera rays are handled for the camera
  background only.
- The active camera is exported as a Luz camera block.
- Blender camera roll is preserved through the named camera `up` vector.
- Invalid zero camera focus distances are replaced with a scene-based fallback;
  DOF aperture is disabled unless Blender provides a valid focus target/distance.
- Blender f-stop DOF is converted to a Luz world-unit lens diameter from
  `camera.lens / aperture_fstop`.
- Area lights become `area_light` blocks. Blender's area-light energy is treated
  as total emitted power and converted to Luz surface intensity with
  `energy / (pi * width * height)`, then multiplied by `--light-power-scale`.
- Point and spot lights become small emissive `point_light` sphere blocks.
  Blender's point-light energy is treated as total emitted power and converted
  to Luz sphere surface intensity with `energy / (4 * pi^2 * radius^2)`, then
  multiplied by `--light-power-scale`. When Blender's light softness is zero,
  `--min-point-light-radius` prevents near-zero export radii from producing
  extreme surface intensity. Exported point and spot lights use `visible=0` so
  their emissive sampling spheres do not render as visible bulbs. Use
  `--min-point-light-radius 0` to preserve the old tiny-light intensity
  conversion.
- Sun lights become `directional_light` blocks. They use
  `--sun-power-scale`, not `--light-power-scale`, because Blender SUN energy is
  directional rather than finite-area power.
- When `--sky atmosphere` is used, the exporter writes an `atmosphere=` line
  whose sun angle is derived from the first exported Blender SUN light. Luz's
  atmosphere model only supports a vertical sun angle, so horizontal rotation is
  discarded.
- Emissive mesh materials stay as emissive mesh objects. Luz importance-samples
  emissive meshes directly.
- Common Blender shader nodes connected to Material Output are mapped into
  Luz material properties, including diffuse, glossy, glass, emission,
  Principled, mix, add, RGB/value, image average color, and color ramp values.
- Image textures connected directly to Principled BSDF Base Color or Diffuse
  BSDF Color are exported as PPM textures, downscaled by `--texture-max-size`,
  and referenced from the generated named material with `texture=...`.
- When a material has multiple Material Output nodes, the exporter prefers the
  output target that matches the scene render engine, such as Cycles or EEVEE.
- Blender material values are written as named material property blocks for
  Luz's scene parser to approximate.

## Current Limits

- Only direct image textures feeding the selected shader's base color are
  exported as texture files. Procedural textures, normal maps, bump maps,
  displacement, and full shader graphs are not exported as texture networks.
- Image texture export uses Luz's built-in PPM texture loader. When no usable
  Base Color image texture is found, image colors can still be approximated by
  averaging a temporary thumbnail, controlled by `--texture-sample-size`.
- Per-face materials are preserved by splitting mesh objects into one OBJ per
  material slot.
- Instancing is baked into separate OBJ files.
- Spot lights are approximations, not physically equivalent Blender lights.
- Animations are not exported.
