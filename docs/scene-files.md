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
| `maxlightbounces` | `maxlightbounces=N` | Maximum recursive light bounces. |
| `gamma` | `gamma=0` or `gamma=1` | Enables gamma correction when set to `1`. |
| `bloom` | `bloom=0` or `bloom=1` | Enables bloom when set to `1`. |
| `denoise` | `denoise=0` or `denoise=1` | Writes a denoised companion image using the NFOR feature-buffer denoiser before bloom, tone mapping, and gamma correction. |
| `denoiseoutputfilename` | `denoiseoutputfilename=PATH` | Optional denoised companion output path. Defaults to `outputfilename` with `_denoised` before the extension. Aliases: `denoiseoutput`, `denoise_output`. |
| `outputfilename` | `outputfilename=PATH` | `.bmp` is appended if no `.bmp` suffix is present. |
| `sky` | `sky=none`, `sky=linear`, or `sky=atmosphere` | Selects background rendering. |
| `background` | `background=(R,G,B)` | Background color used when `sky=none`. Aliases: `backgroundcolor`, `background_color`. |
| `atmosphere` | `atmosphere=SUN,EARTH_RADIUS,ATMOSPHERE_RADIUS,HR,HM,SAMPLES,LIGHT_SAMPLES,STARS` | Only valid after `sky=atmosphere`. |
| `distanceblueness` | `distanceblueness=0` or `distanceblueness=1` | Enables distance blue tinting when set to `1`. |

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
point_light fill {
position=(3,4,5)
radius=0.1
color=(0.45,0.55,1.0)
intensity=1.0
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

Objects except plain `obj=path/to/file.obj` must be followed by a material block and a closing `]`. Plain OBJ meshes use the default material.

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
metallic=0
roughness=0.5
emission=(1.0,0.6,0.3)
emissionStrength=0
}
```

`type=principled` is an approximation for Blender exporter output. Emissive
principled materials become `emissive`, metallic materials become `metal`,
transmissive or alpha-blended materials become `dielectric`, and the rest use
Luz's rough plastic/specular `principled` approximation.

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
`point_light` and `sphere_light` create small emissive spheres. These lights are
still sampled through Luz's emissive-hittable lighting path.

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
