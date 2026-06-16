# Luz Scene Files

Luz scene files use a small line-oriented format. Classic scenes can use only
`[settings]` and `[scene]`:

```text
[settings]
key=value

[scene]
camera=(x,y,z),(dx,dy,dz),fov,aperture,focusDistance
objects{
object=...
}
```

Exporter-friendly scenes can also use `[materials]` and `[meshes]` sections.
Do not put blank lines between blocks inside the same section; a blank line ends
the section.

Blank lines end the current top-level section. Comments are lines whose first character is `#`. Do not put spaces around setting or object keys.

The parser is intentionally strict: unknown lines and malformed values throw an error instead of being silently ignored.

## Settings

| Setting | Format | Notes |
| --- | --- | --- |
| `resolution` | `resolution=WIDTH,HEIGHT` | Width and height must be positive. |
| `samples` | `samples=N` | Rays per pixel. |
| `adaptive` | `adaptive=0` or `adaptive=1` | Enables adaptive per-pixel sampling. When enabled, `samples` is the maximum samples per pixel. Aliases: `adaptivesampling`, `adaptive_sampling`. |
| `adaptiveminsamples` | `adaptiveminsamples=N` | Minimum samples before adaptive stopping can occur. Alias: `adaptive_min_samples`. |
| `adaptivethreshold` | `adaptivethreshold=F` | Relative 95% confidence interval threshold for luminance convergence. Lower values render longer. Alias: `adaptive_threshold`. |
| `adaptivecheckinterval` | `adaptivecheckinterval=N` | Sample interval between adaptive convergence checks. Alias: `adaptive_check_interval`. |
| `maxlightbounces` | `maxlightbounces=N` | Maximum recursive light bounces. |
| `gamma` | `gamma=0` or `gamma=1` | Enables gamma correction when set to `1`. |
| `tonemapping` | `tonemapping=0` or `tonemapping=1` | Enables filmic tone mapping when set to `1`. Alias: `tone_mapping`. |
| `bloom` | `bloom=0` or `bloom=1` | Enables bloom when set to `1`. |
| `exposure` | `exposure=F` | Exposure compensation in stops. `1.0` doubles light before bloom and tone mapping; `-1.0` halves it. |
| `contrast` | `contrast=F` | Display contrast multiplier applied after tone mapping and before gamma correction. `1.0` keeps contrast unchanged. |
| `denoise` | `denoise=0` or `denoise=1` | Writes a denoised companion image using the NFOR feature-buffer denoiser before exposure, bloom, tone mapping, contrast, and gamma correction. |
| `denoiseoutputfilename` | `denoiseoutputfilename=PATH` | Optional denoised companion output path. Defaults to `outputfilename` with `_denoised` before the extension. Must use a `.bmp`, `.png`, or `.tiff` suffix. Aliases: `denoiseoutput`, `denoise_output`. |
| `outputfilename` | `outputfilename=PATH` | `.bmp` is appended if no suffix is present. Explicit suffixes must be `.bmp`, `.png`, or `.tiff`; `.tif` is not accepted. PNG output is 8-bit RGB SDR. TIFF output is uncompressed 32-bit floating-point RGB; disable tone mapping and gamma to preserve scene-linear HDR values above 1.0. |
| `sky` | `sky=none`, `sky=linear`, or `sky=atmosphere` | Selects background rendering. |
| `background` | `background=(R,G,B)` | Background color used when `sky=none`. Aliases: `backgroundcolor`, `background_color`. |
| `atmosphere` | `atmosphere=SUN,EARTH_RADIUS,ATMOSPHERE_RADIUS,HR,HM,SAMPLES,LIGHT_SAMPLES,STARS` | Only valid after `sky=atmosphere`. |
| `distanceblueness` | `distanceblueness=0` or `distanceblueness=1` | Enables distance blue tinting when set to `1`. |

### Adaptive Sampling Notes

Adaptive sampling never exceeds `samples`. It renders at least
`adaptiveminsamples`, then periodically estimates luminance variance and stops a
pixel early only when the configured confidence threshold is met. Dark pixels
that are consistently black can finish quickly, while low-light pixels with rare
bright contributions continue sampling.

### Denoising Notes

`denoise=1` has no hard minimum resolution or sample count, but NFOR needs
enough signal to estimate useful color and feature statistics. One sample per
pixel is not a good quality target: there is no per-pixel variance estimate, so
the denoised image can look almost unchanged or can smooth the wrong details.
Use at least a few samples per pixel for quick previews, and prefer roughly 16+
samples per pixel when judging denoiser quality. Very low resolutions can also
be misleading because each local filter window covers too much of the image.

## Scene

Each scene needs at least one camera:

```text
camera=(x,y,z),(dx,dy,dz),fov,aperture,focusDistance
```

The first vector is the camera position. The second vector is the camera direction.
The `fov` value is horizontal FOV. The `aperture` value is a lens diameter in
Luz world units; use `0` to disable depth of field.

Named camera blocks also support an optional `up` vector:

| Camera Property | Format | Notes |
| --- | --- | --- |
| `position` | `position=(x,y,z)` | Camera origin in Luz world space. |
| `direction` | `direction=(x,y,z)` | Look direction. It does not need to be normalized. |
| `up` | `up=(x,y,z)` | Image-up direction used to preserve camera roll. Defaults to `(0,1,0)`. |
| `fov` | `fov=DEGREES` | Horizontal field of view. |
| `aperture` | `aperture=DIAMETER` | Lens diameter in Luz world units. |
| `focusDistance` | `focusDistance=DISTANCE` | Distance to the focal plane along the camera direction. |

Objects are placed inside an `objects{` block:

```text
objects{
sphere=(0,1,-2),1,material[
lambertian=(0.8,0.2,0.2)
]
}
```

The parser also supports named blocks in `[materials]`, `[meshes]`, and `[scene]`.
This is the preferred target for exporters because it keeps Blender-like object,
material, mesh, camera, and light structure visible in the `.luz` file.

```text
[materials]
material brushed_metal {
type=principled
base_color=(0.75,0.72,0.68)
metallic=1
roughness=0.18
}

[meshes]
mesh helmet_mesh {
file=assets/objects/blender_mandalorian.obj
}

[scene]
camera main {
position=(6.2,3.8,8.2)
direction=(-6.2,-1.54,-8.2)
up=(0,1,0)
fov=46
aperture=0.04
focusDistance=10.5
}
object helmet {
mesh=helmet_mesh
position=(0,0,0)
rotation=(0,0,0)
scale=(1,1,1)
material=brushed_metal
}
area_light key {
position=(-2.5,11.0,1.5)
normal=(0,-1,0)
size=(10,8)
color=(1.0,0.86,0.62)
intensity=3.5
}
directional_light sun {
direction=(0,-1,0)
color=(1.0,0.95,0.8)
intensity=1.0
}
volume room_fog {
shape=box
position=(0,2.5,2.8)
size=(8,5,9)
density=0.05
color=(0.72,0.78,0.9)
anisotropy=0.55
}
point_light fill {
position=(3,4,5)
radius=0.1
color=(0.45,0.55,1.0)
intensity=1.0
visible=0
}
```

## Objects

| Object | Format |
| --- | --- |
| Sphere | `sphere=(x,y,z),radius,material[` |
| Plane | `plane=y,(ox,oy,oz),material[` |
| Rectangle | `rectangle=(x,y,z),(ox,oy,oz),width,height,material[` |
| Triangle | `triangle=(x0,y0,z0),(x1,y1,z1),(x2,y2,z2),material[` |
| Cube | `cube=(x,y,z),(ox,oy,oz),width,height,depth,material[` |
| OBJ mesh | `obj=path/to/file.obj` |
| Transformed OBJ mesh | `obj=path/to/file.obj,(x,y,z),material[` |
| Volume block | `volume name { ... }` |

Objects except plain `obj=path/to/file.obj` must be followed by a material block and a closing `]`. Plain OBJ meshes use the default material.

### Volumes

Volume blocks create constant-density participating media bounded by an internal
sphere or box. They are intended for fog, mist, smoke, colored glass interiors,
and visible light shafts. The boundary is not rendered as a surface unless you
also add a normal object using the same shape.

| Volume Property | Format | Notes |
| --- | --- | --- |
| `shape` | `shape=box` or `shape=sphere` | Defaults to `box`. Alias: `type`. |
| `position` | `position=(x,y,z)` | Center of the volume. Alias: `center`. |
| `size` | `size=(width,height,depth)` | Box dimensions. Alias: `dimensions`; `width`, `height`, and `depth` are also accepted. |
| `radius` | `radius=R` | Sphere radius. |
| `density` | `density=F` | Extinction density. Higher values create thicker fog. Aliases: `extinction`, `sigma_t`. |
| `color` | `color=(r,g,b)` | Scattering albedo/tint when no named phase material is used. Aliases: `albedo`, `scattering_color`. |
| `anisotropy` | `anisotropy=G` | Henyey-Greenstein phase parameter in `[-0.99,0.99]`. Positive values create forward-scattering godrays; `0` uses isotropic scattering. Alias: `g`. |
| `material` | `material=name` | Optional named phase material from `[materials]`. |

```text
volume sun_mist {
shape=sphere
position=(0,2,2)
radius=8
density=0.035
color=(0.85,0.9,1.0)
anisotropy=0.65
}
```

OBJ paths are resolved in this order:

1. Relative to the scene file.
2. Relative to the current working directory.
3. Under `assets/objects/`.
4. Under `objects/`.

## Materials

Each material block must define exactly one material:

| Material | Format |
| --- | --- |
| Lambertian | `lambertian=(r,g,b)` |
| Metal | `metal=(r,g,b),reflectionFuzziness` |
| Dielectric | `dielectric=(r,g,b)` |
| Emissive | `emissive=(r,g,b),lightIntensity` |
| Isotropic phase | `isotropic=(r,g,b)` |
| Henyey-Greenstein phase | `henyey_greenstein=(r,g,b),anisotropy` |

Color channels are floating point values. The renderer normally expects values in the `0.0` to `1.0` range, although emissive intensity is separate.

Named material blocks can use the direct material lines above, or property syntax:

```text
[materials]
material glass {
type=dielectric
color=(1.0,1.0,1.0)
}
material principled_export {
type=principled
base_color=(0.8,0.2,0.1)
texture=textures/albedo.ppm
metallic=0
roughness=0.5
emission=(1.0,0.6,0.3)
emissionStrength=0
}
material warm_fog {
type=phase
color=(1.0,0.86,0.68)
anisotropy=0.6
}
```

Named material property blocks can attach an image texture with `texture=PATH`.
Aliases are `baseColorTexture`, `base_color_texture`, and `albedo`. Texture
paths are resolved like other assets: relative to the scene file, relative to
the current working directory, then under common asset directories including
`textures/` and `assets/textures/`. Luz currently loads PPM `P3` and `P6`
texture files. Textures are sampled with OBJ UV coordinates and multiplied by
the material's base color.

`type=principled` is an approximation for Blender exporter output. Emissive
principled materials become `emissive`, metallic materials become `metal`,
transmissive or alpha-blended materials become `dielectric`, and the rest use
Luz's rough plastic/specular `principled` approximation.

`type=isotropic` and `type=phase`/`type=henyey_greenstein` are intended for
volume blocks. A positive Henyey-Greenstein anisotropy favors forward scattering,
which is the useful control for fog shafts and godrays.

## Named Meshes, Objects, And Lights

Named meshes bind a mesh name to an OBJ file:

```text
[meshes]
mesh suzanne {
file=assets/objects/blender_monkey.obj
}
```

Named object blocks can reference `mesh=NAME` or provide `file=PATH` directly.
OBJ vertices are transformed with `scale`, then `rotation` in degrees around
X/Y/Z, then `position`.
OBJ `vn` normals are used for smooth shading. OBJ `vt` texture coordinates are
used when the selected material has a texture.

```text
object suzanne {
mesh=suzanne
position=(0,1,0)
rotation=(0,45,0)
scale=(1,1,1)
material=matte_red
}
```

`area_light` creates an emissive rectangle and supports arbitrary normals.
`directional_light` creates an infinite light whose `direction` is the direction
light travels, suitable for sun lights. `point_light` and `sphere_light` create
emissive spheres. These lights are still sampled through Luz's emissive-hittable
lighting path. Sphere and point lights also accept `visible=0` to hide the light
surface from camera and shadow rays while keeping it available for direct-light
sampling.

## Minimal Example

```text
[settings]
resolution=300,300
samples=16
maxlightbounces=6
gamma=1
sky=linear

[scene]
camera=(0,1,4),(0,0,-1),60,0,4
objects{
sphere=(0,0,-1),1,material[
lambertian=(0.8,0.2,0.2)
]
plane=-1,(0,1,0),material[
lambertian=(0.8,0.8,0.8)
]
}
```
