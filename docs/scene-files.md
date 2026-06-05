# Luz Scene Files

Luz scene files use a small line-oriented format with two top-level sections:

```text
[settings]
key=value

[scene]
camera=(x,y,z),(dx,dy,dz),fov,aperture,focusDistance
objects{
object=...
}
```

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
| `outputfilename` | `outputfilename=PATH` | `.bmp` is appended if no `.bmp` suffix is present. |
| `sky` | `sky=none`, `sky=linear`, or `sky=atmosphere` | Selects background rendering. |
| `atmosphere` | `atmosphere=SUN,EARTH_RADIUS,ATMOSPHERE_RADIUS,HR,HM,SAMPLES,LIGHT_SAMPLES,STARS` | Only valid after `sky=atmosphere`. |
| `distanceblueness` | `distanceblueness=0` or `distanceblueness=1` | Enables distance blue tinting when set to `1`. |

## Scene

Each scene needs at least one camera:

```text
camera=(x,y,z),(dx,dy,dz),fov,aperture,focusDistance
```

The first vector is the camera position. The second vector is the camera direction.

Objects are placed inside an `objects{` block:

```text
objects{
sphere=(0,1,-2),1,material[
lambertian=(0.8,0.2,0.2)
]
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
