# Blender Exporter

Luz can render Blender scenes through a generated `.luz` scene file and OBJ
mesh files. The exporter runs inside Blender so it can read `.blend` files
through Blender's own Python API.

## Usage

From the repository root:

```sh
"/Applications/Blender.app/Contents/MacOS/Blender" -b scene.blend --python tools/blender_export_luz.py -- --output exports/scene.luz
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
--light-power-scale N         Convert Blender light energy to Luz intensity. Defaults to 0.01.
--camera-aperture N           Override Luz camera aperture.
```

## What Gets Exported

- Mesh objects are evaluated with modifiers, triangulated, split by material
  slot, and written as simple Luz-compatible OBJ files.
- Blender world transforms are baked into OBJ vertices for reliable placement.
- Blender's Z-up world is converted to Luz's Y-up world.
- Evaluated Blender corner normals are exported as OBJ `vn` data so Luz can
  preserve smooth shading.
- Blender World background color is exported as `sky=none` plus
  `background=(R,G,B)` unless `--sky` overrides the sky mode.
- The active camera is exported as a Luz camera block.
- Blender camera roll is preserved through the named camera `up` vector.
- Invalid zero camera focus distances are replaced with a scene-based fallback;
  DOF aperture is disabled unless Blender provides a valid focus target/distance.
- Blender f-stop DOF is converted to a Luz world-unit lens diameter from
  `camera.lens / aperture_fstop`.
- Area lights become `area_light` blocks.
- Point and spot lights become small emissive `point_light` sphere blocks.
- Sun lights are approximated as distant area lights.
- Emissive mesh materials stay as emissive mesh objects. Luz importance-samples
  emissive meshes directly.
- Common Blender shader nodes connected to Material Output are mapped into
  Luz material properties, including diffuse, glossy, glass, emission,
  Principled, mix, add, RGB/value, image average color, and color ramp values.
- Blender material values are written as `type=principled` named materials for
  Luz's scene parser to approximate.

## Current Limits

- UVs, procedural textures, and full shader graphs are not exported. Image
  texture colors are approximated by a sampled average.
- Per-face materials are preserved by splitting mesh objects into one OBJ per
  material slot.
- Instancing is baked into separate OBJ files.
- Spot and sun lights are approximations, not physically equivalent Blender
  lights.
- Animations are not exported.
