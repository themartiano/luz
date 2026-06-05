#!/usr/bin/env python3
"""
Export the current Blender scene to a Luz .luz file plus simple OBJ meshes.

Run from Blender:
  blender -b scene.blend --python tools/blender_export_luz.py -- --output exports/scene.luz
"""

from __future__ import annotations

import argparse
import math
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

try:
	import bpy
	from mathutils import Vector
except ImportError as error:
	raise SystemExit("This exporter must be run inside Blender with --python.") from error


@dataclass
class LuzMaterial:
	name: str
	material_type: str
	base_color: tuple[float, float, float]
	metallic: float
	roughness: float
	alpha: float
	transmission: float
	emission_color: tuple[float, float, float]
	emission_strength: float


@dataclass
class LuzMesh:
	name: str
	file_path: Path
	scene_file_path: str
	material_name: str
	object_name: str
	triangle_count: int


@dataclass
class LuzCamera:
	name: str
	position: Vector
	direction: Vector
	up: Vector
	fov: float
	aperture: float
	focus_distance: float


@dataclass
class LuzLight:
	kind: str
	name: str
	position: Vector
	color: tuple[float, float, float]
	intensity: float
	normal: Vector | None = None
	width: float = 1.0
	height: float = 1.0
	radius: float = 0.1


class Bounds:
	def __init__(self) -> None:
		self.minimum: Vector | None = None
		self.maximum: Vector | None = None

	def include(self, point: Vector) -> None:
		if self.minimum is None or self.maximum is None:
			self.minimum = point.copy()
			self.maximum = point.copy()
			return

		self.minimum.x = min(self.minimum.x, point.x)
		self.minimum.y = min(self.minimum.y, point.y)
		self.minimum.z = min(self.minimum.z, point.z)
		self.maximum.x = max(self.maximum.x, point.x)
		self.maximum.y = max(self.maximum.y, point.y)
		self.maximum.z = max(self.maximum.z, point.z)

	def center(self) -> Vector:
		if self.minimum is None or self.maximum is None:
			return Vector((0.0, 0.75, 0.0))
		return (self.minimum + self.maximum) * 0.5

	def radius(self) -> float:
		if self.minimum is None or self.maximum is None:
			return 1.0
		return max((self.maximum - self.minimum).length * 0.5, 1.0)


def parse_args(argv: list[str]) -> argparse.Namespace:
	if "--" in argv:
		argv = argv[argv.index("--") + 1:]
	else:
		argv = []

	parser = argparse.ArgumentParser(description="Export a Blender scene to Luz.")
	parser.add_argument("--blend", help="Optional .blend file to open before exporting.")
	parser.add_argument("-o", "--output", required=True, help="Output .luz scene file.")
	parser.add_argument("--mesh-dir", default="meshes", help="OBJ output directory, relative to the .luz file by default.")
	parser.add_argument("--selected-only", action="store_true", help="Export selected objects only.")
	parser.add_argument("--include-hidden", action="store_true", help="Include objects hidden from render.")
	parser.add_argument("--resolution", help="Override resolution as WIDTHxHEIGHT.")
	parser.add_argument("--samples", type=int, help="Override Luz samples per pixel.")
	parser.add_argument("--max-light-bounces", type=int, help="Override Luz max light bounces.")
	parser.add_argument("--sky", choices=("linear", "none", "atmosphere"), help="Override Luz sky mode.")
	parser.add_argument("--render-output", help="Luz render output filename. .bmp is appended by Luz when omitted.")
	parser.add_argument("--global-scale", type=float, default=1.0, help="Scale all exported positions and mesh vertices.")
	parser.add_argument("--light-power-scale", type=float, default=0.01, help="Multiplier from Blender light energy to Luz intensity.")
	parser.add_argument("--sun-distance", type=float, default=100.0, help="Distance used when approximating sun lights.")
	parser.add_argument("--sun-size", type=float, default=40.0, help="Area size used when approximating sun lights.")
	parser.add_argument("--camera-aperture", type=float, help="Override Luz camera lens diameter in world units.")
	parser.add_argument("--default-focus-distance", type=float, default=10.0, help="Fallback focus distance in Blender units.")
	return parser.parse_args(argv)


def slugify(value: str, fallback: str) -> str:
	slug = re.sub(r"[^A-Za-z0-9_]+", "_", value).strip("_").lower()
	if not slug:
		slug = fallback
	if slug[0].isdigit():
		slug = f"{fallback}_{slug}"
	return slug


def unique_name(base: str, used: set[str]) -> str:
	name = base
	index = 2
	while name in used:
		name = f"{base}_{index}"
		index += 1
	used.add(name)
	return name


def fmt_float(value: float) -> str:
	if abs(value) < 1e-12:
		value = 0.0
	return f"{value:.8g}"


def fmt_vector(value: Vector) -> str:
	return f"({fmt_float(value.x)},{fmt_float(value.y)},{fmt_float(value.z)})"


def fmt_color(value: tuple[float, float, float]) -> str:
	return f"({fmt_float(value[0])},{fmt_float(value[1])},{fmt_float(value[2])})"


def blender_point_to_luz(point: Vector, global_scale: float) -> Vector:
	return Vector((point.x * global_scale, point.z * global_scale, -point.y * global_scale))


def blender_vector_to_luz(vector: Vector) -> Vector:
	result = Vector((vector.x, vector.z, -vector.y))
	if result.length == 0.0:
		return Vector((0.0, 0.0, -1.0))
	return result.normalized()


def blender_normal_to_luz(normal: Vector) -> Vector:
	return blender_vector_to_luz(normal)


def color_from_value(value: object, fallback: tuple[float, float, float]) -> tuple[float, float, float]:
	try:
		return (float(value[0]), float(value[1]), float(value[2]))  # type: ignore[index]
	except (TypeError, IndexError, ValueError):
		return fallback


def scalar_from_value(value: object, fallback: float) -> float:
	try:
		return float(value)  # type: ignore[arg-type]
	except (TypeError, ValueError):
		return fallback


def blend_color(
	first: tuple[float, float, float],
	second: tuple[float, float, float],
	factor: float,
) -> tuple[float, float, float]:
	factor = max(0.0, min(1.0, factor))
	return (
		first[0] * (1.0 - factor) + second[0] * factor,
		first[1] * (1.0 - factor) + second[1] * factor,
		first[2] * (1.0 - factor) + second[2] * factor,
	)


def blend_scalar(first: float, second: float, factor: float) -> float:
	factor = max(0.0, min(1.0, factor))
	return first * (1.0 - factor) + second * factor


def average_image_color(image: object | None) -> tuple[float, float, float] | None:
	if image is None:
		return None
	try:
		pixels = image.pixels
		pixel_count = int(len(pixels) / 4)
	except (AttributeError, TypeError, ValueError):
		return None
	if pixel_count <= 0:
		return None

	step = max(int(pixel_count / 1024), 1)
	red = 0.0
	green = 0.0
	blue = 0.0
	samples = 0
	for pixel_index in range(0, pixel_count, step):
		offset = pixel_index * 4
		red += float(pixels[offset])
		green += float(pixels[offset + 1])
		blue += float(pixels[offset + 2])
		samples += 1
	if samples == 0:
		return None
	return (red / samples, green / samples, blue / samples)


def node_socket(node: object, name: str) -> object | None:
	try:
		return node.inputs.get(name)  # type: ignore[attr-defined]
	except AttributeError:
		return None


def socket_default(socket: object | None, fallback: object) -> object:
	if socket is not None and hasattr(socket, "default_value"):
		return socket.default_value
	return fallback


def resolve_output_value(output_socket: object, fallback: object, visited: set[object], depth: int) -> object:
	node = getattr(output_socket, "node", None)
	if node is None or depth > 16 or node in visited:
		return socket_default(output_socket, fallback)

	visited.add(node)
	node_type = getattr(node, "type", "")
	output_name = getattr(output_socket, "name", "")

	if node_type == "RGB":
		return socket_default(node.outputs.get("Color"), socket_default(output_socket, fallback))  # type: ignore[attr-defined]
	if node_type == "VALUE":
		return socket_default(node.outputs.get("Value"), socket_default(output_socket, fallback))  # type: ignore[attr-defined]
	if node_type == "TEX_IMAGE" and output_name == "Color":
		return average_image_color(getattr(node, "image", None)) or socket_default(output_socket, fallback)
	if node_type == "VALTORGB" and output_name == "Color":
		factor = scalar_from_value(resolve_socket_value(node_socket(node, "Fac"), 0.5, visited, depth + 1), 0.5)
		try:
			return node.color_ramp.evaluate(max(0.0, min(1.0, factor)))
		except Exception:
			return socket_default(output_socket, fallback)
	if node_type in {"MIX_RGB", "MIX"} and output_name in {"Color", "Result"}:
		factor = scalar_from_value(resolve_socket_value(node_socket(node, "Fac"), 0.5, visited, depth + 1), 0.5)
		color_a = color_from_value(
			resolve_socket_value(node_socket(node, "Color1") or node_socket(node, "A"), fallback, visited, depth + 1),
			color_from_value(fallback, (0.0, 0.0, 0.0)),
		)
		color_b = color_from_value(
			resolve_socket_value(node_socket(node, "Color2") or node_socket(node, "B"), fallback, visited, depth + 1),
			color_from_value(fallback, (0.0, 0.0, 0.0)),
		)
		return blend_color(color_a, color_b, factor)
	if node_type == "MATH" and output_name in {"Value", "Result"}:
		first = scalar_from_value(resolve_socket_value(node_socket(node, "Value"), 0.0, visited, depth + 1), 0.0)
		second = scalar_from_value(resolve_socket_value(node_socket(node, "Value_001"), 0.0, visited, depth + 1), 0.0)
		operation = getattr(node, "operation", "")
		if operation == "ADD":
			return first + second
		if operation == "SUBTRACT":
			return first - second
		if operation == "MULTIPLY":
			return first * second
		if operation == "DIVIDE":
			return first / second if abs(second) > 1e-12 else first
		if operation == "POWER":
			return first ** second
		if operation == "MINIMUM":
			return min(first, second)
		if operation == "MAXIMUM":
			return max(first, second)
		return first
	if node_type in {"EMISSION", "BSDF_DIFFUSE", "BSDF_GLOSSY", "BSDF_PRINCIPLED"}:
		if output_name in {"Color", "BSDF", "Emission"}:
			return resolve_socket_value(
				node_socket(node, "Base Color") or node_socket(node, "Color") or node_socket(node, "Emission Color"),
				fallback,
				visited,
				depth + 1,
			)
		if output_name == "Strength":
			return resolve_socket_value(node_socket(node, "Strength"), fallback, visited, depth + 1)

	return socket_default(output_socket, fallback)


def resolve_socket_value(socket: object | None, fallback: object, visited: set[object] | None = None, depth: int = 0) -> object:
	if socket is None:
		return fallback
	if visited is None:
		visited = set()
	try:
		if socket.links:
			return resolve_output_value(socket.links[0].from_socket, fallback, visited, depth + 1)
	except AttributeError:
		pass
	return socket_default(socket, fallback)


def node_input_value(node: object, names: tuple[str, ...], fallback: object) -> object:
	for name in names:
		socket = node_socket(node, name)
		if socket is not None:
			return resolve_socket_value(socket, fallback)
	return fallback


def socket_default_value(node: object, names: tuple[str, ...], fallback: object) -> object:
	for name in names:
		socket = node_socket(node, name)
		if socket is not None:
			return resolve_socket_value(socket, fallback)
	return fallback


def find_principled_node(material: object) -> object | None:
	if not material or not getattr(material, "use_nodes", False) or not getattr(material, "node_tree", None):
		return None
	for node in material.node_tree.nodes:
		if node.type == "BSDF_PRINCIPLED":
			return node
	return None


def emission_from_node(node: object, visited: set[object]) -> tuple[tuple[float, float, float], float] | None:
	if node in visited:
		return None
	visited.add(node)

	if node.type == "EMISSION":
		color = color_from_value(socket_default_value(node, ("Color",), (1.0, 1.0, 1.0)), (1.0, 1.0, 1.0))
		strength = scalar_from_value(socket_default_value(node, ("Strength",), 1.0), 1.0)
		if strength > 0.0:
			return (color, strength)
		return None

	if node.type == "BSDF_PRINCIPLED":
		color = color_from_value(
			socket_default_value(node, ("Emission Color", "Emission"), (1.0, 1.0, 1.0)),
			(1.0, 1.0, 1.0),
		)
		strength = scalar_from_value(socket_default_value(node, ("Emission Strength",), 0.0), 0.0)
		if strength > 0.0:
			return (color, strength)

	for socket in node.inputs:
		for link in socket.links:
			emission = emission_from_node(link.from_node, visited)
			if emission:
				return emission
	return None


def material_output_surface_node(material: object) -> object | None:
	if not material or not getattr(material, "use_nodes", False) or not getattr(material, "node_tree", None):
		return None

	for node in material.node_tree.nodes:
		if node.type != "OUTPUT_MATERIAL":
			continue
		surface = node.inputs.get("Surface")
		if surface and surface.links:
			return surface.links[0].from_node
	return None


def material_emission(material: object) -> tuple[tuple[float, float, float], float] | None:
	surface_node = material_output_surface_node(material)
	if surface_node:
		emission = emission_from_node(surface_node, set())
		if emission:
			return emission

	if material and getattr(material, "use_nodes", False) and getattr(material, "node_tree", None):
		for node in material.node_tree.nodes:
			if node.type == "EMISSION":
				emission = emission_from_node(node, set())
				if emission:
					return emission
	return None


def material_defaults(
	material_type: str,
	base_color: tuple[float, float, float],
	metallic: float,
	roughness: float,
	alpha: float,
	transmission: float,
	emission_color: tuple[float, float, float],
	emission_strength: float,
) -> dict[str, object]:
	return {
		"material_type": material_type,
		"base_color": base_color,
		"metallic": metallic,
		"roughness": roughness,
		"alpha": alpha,
		"transmission": transmission,
		"emission_color": emission_color,
		"emission_strength": emission_strength,
	}


def blend_material_values(
	first: dict[str, object],
	second: dict[str, object],
	factor: float,
) -> dict[str, object]:
	return {
		"material_type": first["material_type"] if first["material_type"] == second["material_type"] else "principled",
		"base_color": blend_color(first["base_color"], second["base_color"], factor),  # type: ignore[arg-type]
		"metallic": blend_scalar(float(first["metallic"]), float(second["metallic"]), factor),
		"roughness": blend_scalar(float(first["roughness"]), float(second["roughness"]), factor),
		"alpha": blend_scalar(float(first["alpha"]), float(second["alpha"]), factor),
		"transmission": blend_scalar(float(first["transmission"]), float(second["transmission"]), factor),
		"emission_color": blend_color(first["emission_color"], second["emission_color"], factor),  # type: ignore[arg-type]
		"emission_strength": blend_scalar(float(first["emission_strength"]), float(second["emission_strength"]), factor),
	}


def linked_shader_nodes(node: object) -> list[object]:
	nodes: list[object] = []
	for socket in getattr(node, "inputs", []):
		if socket.name == "Fac":
			continue
		for link in socket.links:
			nodes.append(link.from_node)
	return nodes


def material_values_from_shader_node(
	node: object | None,
	defaults: dict[str, object],
	visited: set[object] | None = None,
) -> dict[str, object]:
	if node is None:
		return defaults
	if visited is None:
		visited = set()
	if node in visited:
		return defaults
	visited.add(node)

	node_type = getattr(node, "type", "")
	values = dict(defaults)

	if node_type == "BSDF_PRINCIPLED":
		values["material_type"] = "principled"
		values["base_color"] = color_from_value(socket_default_value(node, ("Base Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["metallic"] = scalar_from_value(socket_default_value(node, ("Metallic",), values["metallic"]), float(values["metallic"]))
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), values["roughness"]), float(values["roughness"]))
		values["alpha"] = scalar_from_value(socket_default_value(node, ("Alpha",), values["alpha"]), float(values["alpha"]))
		values["transmission"] = scalar_from_value(
			socket_default_value(node, ("Transmission Weight", "Transmission"), values["transmission"]),
			float(values["transmission"]),
		)
		values["emission_color"] = color_from_value(
			socket_default_value(node, ("Emission Color", "Emission"), values["emission_color"]),
			values["emission_color"],  # type: ignore[arg-type]
		)
		values["emission_strength"] = scalar_from_value(
			socket_default_value(node, ("Emission Strength",), values["emission_strength"]),
			float(values["emission_strength"]),
		)
		return values

	if node_type == "BSDF_DIFFUSE":
		values["material_type"] = "lambertian"
		values["base_color"] = color_from_value(socket_default_value(node, ("Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), 1.0), 1.0)
		return values

	if node_type in {"BSDF_GLOSSY", "BSDF_ANISOTROPIC"}:
		values["material_type"] = "principled"
		values["base_color"] = color_from_value(socket_default_value(node, ("Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), values["roughness"]), float(values["roughness"]))
		return values

	if node_type in {"BSDF_GLASS", "BSDF_TRANSLUCENT", "BSDF_TRANSPARENT"}:
		values["material_type"] = "dielectric"
		values["base_color"] = color_from_value(socket_default_value(node, ("Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), values["roughness"]), float(values["roughness"]))
		values["transmission"] = 1.0
		if node_type == "BSDF_TRANSPARENT":
			values["alpha"] = 0.0
		return values

	if node_type == "EMISSION":
		values["material_type"] = "emissive"
		values["emission_color"] = color_from_value(socket_default_value(node, ("Color",), values["emission_color"]), values["emission_color"])  # type: ignore[arg-type]
		values["emission_strength"] = scalar_from_value(socket_default_value(node, ("Strength",), 1.0), 1.0)
		values["base_color"] = values["emission_color"]
		return values

	if node_type == "MIX_SHADER":
		nodes = linked_shader_nodes(node)
		if len(nodes) >= 2:
			factor = scalar_from_value(socket_default_value(node, ("Fac",), 0.5), 0.5)
			first = material_values_from_shader_node(nodes[0], defaults, visited.copy())
			second = material_values_from_shader_node(nodes[1], defaults, visited.copy())
			return blend_material_values(first, second, factor)

	if node_type == "ADD_SHADER":
		nodes = linked_shader_nodes(node)
		if len(nodes) >= 2:
			first = material_values_from_shader_node(nodes[0], defaults, visited.copy())
			second = material_values_from_shader_node(nodes[1], defaults, visited.copy())
			values = blend_material_values(first, second, 0.5)
			values["emission_strength"] = float(first["emission_strength"]) + float(second["emission_strength"])
			if float(second["emission_strength"]) > float(first["emission_strength"]):
				values["emission_color"] = second["emission_color"]
			else:
				values["emission_color"] = first["emission_color"]
			return values

	for child in linked_shader_nodes(node):
		child_values = material_values_from_shader_node(child, defaults, visited.copy())
		if child_values != defaults:
			return child_values
	return values


def export_material(material: object | None, used_names: set[str], registry: dict[str, LuzMaterial]) -> str:
	key = material.name_full if material else "__luz_default_material__"
	if key in registry:
		return registry[key].name

	name = unique_name(slugify(material.name if material else "default_material", "material"), used_names)
	material_type = "principled"
	base_color = (0.6, 0.6, 0.6)
	metallic = 0.0
	roughness = 0.5
	alpha = 1.0
	transmission = 0.0
	emission_color = (1.0, 1.0, 1.0)
	emission_strength = 0.0

	if material:
		base_color = color_from_value(getattr(material, "diffuse_color", base_color), base_color)
		try:
			alpha = float(material.diffuse_color[3])
		except (AttributeError, IndexError, TypeError, ValueError):
			alpha = 1.0

		principled = find_principled_node(material)
		if principled:
			base_color = color_from_value(node_input_value(principled, ("Base Color",), base_color), base_color)
			metallic = scalar_from_value(node_input_value(principled, ("Metallic",), metallic), metallic)
			roughness = scalar_from_value(node_input_value(principled, ("Roughness",), roughness), roughness)
			alpha = scalar_from_value(node_input_value(principled, ("Alpha",), alpha), alpha)
			transmission = scalar_from_value(
				node_input_value(principled, ("Transmission Weight", "Transmission"), transmission),
				transmission,
			)
			emission_color = color_from_value(
				node_input_value(principled, ("Emission Color", "Emission"), emission_color),
				emission_color,
			)
			emission_strength = scalar_from_value(
				node_input_value(principled, ("Emission Strength",), emission_strength),
				emission_strength,
			)

		surface_node = material_output_surface_node(material)
		if surface_node:
			values = material_values_from_shader_node(
				surface_node,
				material_defaults(
					material_type,
					base_color,
					metallic,
					roughness,
					alpha,
					transmission,
					emission_color,
					emission_strength,
				),
			)
			material_type = str(values["material_type"])
			base_color = values["base_color"]  # type: ignore[assignment]
			metallic = float(values["metallic"])
			roughness = float(values["roughness"])
			alpha = float(values["alpha"])
			transmission = float(values["transmission"])
			emission_color = values["emission_color"]  # type: ignore[assignment]
			emission_strength = float(values["emission_strength"])

		emission = material_emission(material)
		if emission:
			emission_color = emission[0]
			emission_strength = emission[1]

	registry[key] = LuzMaterial(
		name=name,
		material_type=material_type,
		base_color=base_color,
		metallic=metallic,
		roughness=roughness,
		alpha=alpha,
		transmission=transmission,
		emission_color=emission_color,
		emission_strength=emission_strength,
	)
	return name


def material_for_slot(object_: object, material_index: int) -> object | None:
	if material_index < 0 or material_index >= len(object_.material_slots):
		return None
	return object_.material_slots[material_index].material


def should_export_object(object_: object, args: argparse.Namespace) -> bool:
	if args.selected_only and not object_.select_get():
		return False
	if not args.include_hidden and getattr(object_, "hide_render", False):
		return False
	return True


def evaluated_mesh_for_object(object_: object, depsgraph: object) -> object:
	evaluated_object = object_.evaluated_get(depsgraph)
	try:
		mesh = evaluated_object.to_mesh(preserve_all_data_layers=False, depsgraph=depsgraph)
	except TypeError:
		mesh = evaluated_object.to_mesh()
	return evaluated_object, mesh


def release_evaluated_mesh(evaluated_object: object) -> None:
	if hasattr(evaluated_object, "to_mesh_clear"):
		evaluated_object.to_mesh_clear()


def loop_normal(mesh: object, loop_index: int, normal_matrix: object) -> Vector:
	try:
		blender_normal = mesh.corner_normals[loop_index].vector.copy()
	except (AttributeError, IndexError):
		blender_normal = mesh.loops[loop_index].normal.copy()

	world_normal = normal_matrix @ blender_normal
	return blender_normal_to_luz(world_normal)


ObjCorner = tuple[Vector, Vector]
ObjTriangle = tuple[ObjCorner, ObjCorner, ObjCorner]


def write_obj(mesh_path: Path, triangles: list[ObjTriangle]) -> None:
	mesh_path.parent.mkdir(parents=True, exist_ok=True)
	with mesh_path.open("w", encoding="utf-8") as stream:
		stream.write("# Generated by tools/blender_export_luz.py\n")
		for triangle in triangles:
			for vertex, _normal in triangle:
				stream.write(f"v {fmt_float(vertex.x)} {fmt_float(vertex.y)} {fmt_float(vertex.z)}\n")
		for triangle in triangles:
			for _vertex, normal in triangle:
				stream.write(f"vn {fmt_float(normal.x)} {fmt_float(normal.y)} {fmt_float(normal.z)}\n")
		for index in range(len(triangles)):
			first = (index * 3) + 1
			stream.write(f"f {first}//{first} {first + 1}//{first + 1} {first + 2}//{first + 2}\n")


def export_meshes(
	args: argparse.Namespace,
	output_path: Path,
	mesh_dir: Path,
	materials: dict[str, LuzMaterial],
	used_material_names: set[str],
	bounds: Bounds,
) -> list[LuzMesh]:
	depsgraph = bpy.context.evaluated_depsgraph_get()
	meshes: list[LuzMesh] = []
	used_mesh_names: set[str] = set()
	used_object_names: set[str] = set()

	for object_ in bpy.context.scene.objects:
		if object_.type != "MESH" or not should_export_object(object_, args):
			continue

		evaluated_object, mesh = evaluated_mesh_for_object(object_, depsgraph)
		try:
			if hasattr(mesh, "calc_normals_split"):
				mesh.calc_normals_split()
			mesh.calc_loop_triangles()
			world_matrix = evaluated_object.matrix_world.copy()
			try:
				normal_matrix = world_matrix.to_3x3().inverted().transposed()
			except Exception:
				normal_matrix = world_matrix.to_3x3()
			object_triangles_by_material: dict[int, list[ObjTriangle]] = {}

			for loop_triangle in mesh.loop_triangles:
				material_index = loop_triangle.material_index
				triangle_corners: list[ObjCorner] = []
				for loop_index, vertex_index in zip(loop_triangle.loops, loop_triangle.vertices):
					blender_world = world_matrix @ mesh.vertices[vertex_index].co
					luz_vertex = blender_point_to_luz(blender_world, args.global_scale)
					luz_normal = loop_normal(mesh, loop_index, normal_matrix)
					bounds.include(luz_vertex)
					triangle_corners.append((luz_vertex, luz_normal))
				object_triangles_by_material.setdefault(material_index, []).append(
					(triangle_corners[0], triangle_corners[1], triangle_corners[2])
				)

			for material_index, triangles in object_triangles_by_material.items():
				if not triangles:
					continue

				material = material_for_slot(object_, material_index)
				material_name = export_material(material, used_material_names, materials)
				mesh_base = slugify(f"{object_.name}_{material_name}", "mesh")
				mesh_name = unique_name(mesh_base, used_mesh_names)
				object_name = unique_name(slugify(f"{object_.name}_{material_name}", "object"), used_object_names)
				mesh_path = mesh_dir / f"{mesh_name}.obj"
				scene_file_path = Path(os.path.relpath(mesh_path, output_path.parent)).as_posix()

				write_obj(mesh_path, triangles)
				meshes.append(
					LuzMesh(
						name=mesh_name,
						file_path=mesh_path,
						scene_file_path=scene_file_path,
						material_name=material_name,
						object_name=object_name,
						triangle_count=len(triangles),
					)
				)
		finally:
			release_evaluated_mesh(evaluated_object)

	return meshes


def export_camera(args: argparse.Namespace, bounds: Bounds) -> LuzCamera:
	scene = bpy.context.scene
	camera_object = scene.camera

	if camera_object and camera_object.type == "CAMERA":
		camera_data = camera_object.data
		position = blender_point_to_luz(camera_object.matrix_world.translation, args.global_scale)
		direction = blender_vector_to_luz(camera_object.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0)))
		up = blender_vector_to_luz(camera_object.matrix_world.to_quaternion() @ Vector((0.0, 1.0, 0.0)))
		fov = math.degrees(camera_data.angle_x)

		use_dof = getattr(camera_data.dof, "use_dof", False)
		focus_distance = float(args.default_focus_distance) * args.global_scale
		has_valid_blender_focus = False
		if use_dof:
			focus_object = getattr(camera_data.dof, "focus_object", None)
			if focus_object:
				focus_position = blender_point_to_luz(focus_object.matrix_world.translation, args.global_scale)
				focus_distance = (focus_position - position).length
				has_valid_blender_focus = focus_distance > 1e-6
			else:
				focus_distance = float(camera_data.dof.focus_distance) * args.global_scale
				has_valid_blender_focus = focus_distance > 1e-6

		if focus_distance <= 1e-6:
			center_distance = (bounds.center() - position).dot(direction)
			if center_distance > 1e-6:
				focus_distance = center_distance
			else:
				focus_distance = max(float(args.default_focus_distance) * args.global_scale, 1.0)

		if args.camera_aperture is not None:
			aperture = args.camera_aperture
		elif use_dof and has_valid_blender_focus and getattr(camera_data.dof, "aperture_fstop", 0.0) > 0.0:
			aperture = (float(camera_data.lens) / 1000.0) / float(camera_data.dof.aperture_fstop)
			aperture *= args.global_scale
		else:
			aperture = 0.0

		return LuzCamera("main", position, direction, up, fov, aperture, focus_distance)

	center = bounds.center()
	radius = bounds.radius()
	position = Vector((center.x, center.y + radius * 0.25, center.z + radius * 3.0))
	direction = center - position
	focus_distance = max(direction.length, 1.0)
	return LuzCamera("main", position, direction.normalized(), Vector((0.0, 1.0, 0.0)), 50.0, 0.0, focus_distance)


def light_color(light_data: object) -> tuple[float, float, float]:
	return color_from_value(getattr(light_data, "color", (1.0, 1.0, 1.0)), (1.0, 1.0, 1.0))


def export_lights(args: argparse.Namespace, bounds: Bounds) -> list[LuzLight]:
	lights: list[LuzLight] = []
	used_names: set[str] = set()
	center = bounds.center()

	for object_ in bpy.context.scene.objects:
		if object_.type != "LIGHT" or not should_export_object(object_, args):
			continue

		data = object_.data
		name = unique_name(slugify(object_.name, "light"), used_names)
		position = blender_point_to_luz(object_.matrix_world.translation, args.global_scale)
		direction = blender_vector_to_luz(object_.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0)))
		color = light_color(data)
		intensity = max(float(getattr(data, "energy", 1.0)) * args.light_power_scale, 0.0)

		if data.type == "AREA":
			width = float(getattr(data, "size", 1.0)) * args.global_scale
			height = width
			if getattr(data, "shape", "") in {"RECTANGLE", "ELLIPSE"}:
				height = float(getattr(data, "size_y", width)) * args.global_scale
			lights.append(
				LuzLight(
					kind="area_light",
					name=name,
					position=position,
					normal=direction,
					width=max(width, 1e-6),
					height=max(height, 1e-6),
					color=color,
					intensity=intensity,
				)
			)
		elif data.type == "SUN":
			sun_distance = args.sun_distance * args.global_scale
			sun_size = args.sun_size * args.global_scale
			lights.append(
				LuzLight(
					kind="area_light",
					name=name,
					position=center - (direction * sun_distance),
					normal=direction,
					width=max(sun_size, 1e-6),
					height=max(sun_size, 1e-6),
					color=color,
					intensity=intensity,
				)
			)
		else:
			radius = max(float(getattr(data, "shadow_soft_size", 0.1)) * args.global_scale, 1e-6)
			lights.append(
				LuzLight(
					kind="point_light",
					name=name,
					position=position,
					radius=radius,
					color=color,
					intensity=intensity,
				)
			)

	return lights


def world_surface_node(world: object) -> object | None:
	if not world or not getattr(world, "use_nodes", False) or not getattr(world, "node_tree", None):
		return None
	for node in world.node_tree.nodes:
		if node.type != "OUTPUT_WORLD":
			continue
		surface = node.inputs.get("Surface")
		if surface and surface.links:
			return surface.links[0].from_node
	return None


def export_world_background() -> tuple[float, float, float]:
	world = bpy.context.scene.world
	default_color = (0.0, 0.0, 0.0)
	if not world:
		return default_color

	default_color = color_from_value(getattr(world, "color", default_color), default_color)
	background_node = world_surface_node(world)
	if background_node and background_node.type == "BACKGROUND":
		color = color_from_value(socket_default_value(background_node, ("Color",), default_color), default_color)
		strength = scalar_from_value(socket_default_value(background_node, ("Strength",), 1.0), 1.0)
		return (
			max(color[0] * strength, 0.0),
			max(color[1] * strength, 0.0),
			max(color[2] * strength, 0.0),
		)
	return default_color


def scene_resolution(args: argparse.Namespace) -> tuple[int, int]:
	if args.resolution:
		parts = args.resolution.lower().split("x")
		if len(parts) != 2:
			raise SystemExit("--resolution must use WIDTHxHEIGHT.")
		return int(parts[0]), int(parts[1])

	render = bpy.context.scene.render
	scale = float(render.resolution_percentage) / 100.0
	return max(int(render.resolution_x * scale), 1), max(int(render.resolution_y * scale), 1)


def scene_samples(args: argparse.Namespace) -> int:
	if args.samples:
		return args.samples
	cycles = getattr(bpy.context.scene, "cycles", None)
	if cycles and getattr(cycles, "samples", 0) > 0:
		return int(cycles.samples)
	return 64


def scene_bounces(args: argparse.Namespace) -> int:
	if args.max_light_bounces is not None:
		return args.max_light_bounces
	cycles = getattr(bpy.context.scene, "cycles", None)
	if cycles and getattr(cycles, "max_bounces", 0) >= 0:
		return int(cycles.max_bounces)
	return 8


def write_luz_scene(
	args: argparse.Namespace,
	output_path: Path,
	materials: dict[str, LuzMaterial],
	meshes: list[LuzMesh],
	camera: LuzCamera,
	lights: list[LuzLight],
	background_color: tuple[float, float, float],
) -> None:
	width, height = scene_resolution(args)
	render_output = args.render_output
	if not render_output:
		render_output = str(output_path.with_suffix("")) + "_render"
	sky = args.sky or "none"

	lines: list[str] = []
	lines.extend(
		[
			"[settings]",
			f"resolution={width},{height}",
			f"samples={scene_samples(args)}",
			f"maxlightbounces={scene_bounces(args)}",
			"gamma=1",
			"bloom=1",
			f"sky={sky}",
			f"background={fmt_color(background_color)}",
			f"outputfilename={render_output}",
			"",
		]
	)

	if materials:
		lines.append("[materials]")
		for material in materials.values():
			lines.extend(
				[
					f"material {material.name} {{",
					f"type={material.material_type}",
					f"base_color={fmt_color(material.base_color)}",
					f"metallic={fmt_float(material.metallic)}",
					f"roughness={fmt_float(material.roughness)}",
					f"alpha={fmt_float(material.alpha)}",
					f"transmission={fmt_float(material.transmission)}",
					f"emission={fmt_color(material.emission_color)}",
					f"emissionStrength={fmt_float(material.emission_strength)}",
					"}",
				]
			)
		lines.append("")

	if meshes:
		lines.append("[meshes]")
		for mesh in meshes:
			lines.extend(
				[
					f"mesh {mesh.name} {{",
					f"file={mesh.scene_file_path}",
					"}",
				]
			)
		lines.append("")

	lines.append("[scene]")
	lines.extend(
		[
			f"camera {camera.name} {{",
			f"position={fmt_vector(camera.position)}",
			f"direction={fmt_vector(camera.direction)}",
			f"up={fmt_vector(camera.up)}",
			f"fov={fmt_float(camera.fov)}",
			f"aperture={fmt_float(camera.aperture)}",
			f"focusDistance={fmt_float(camera.focus_distance)}",
			"}",
		]
	)

	for mesh in meshes:
		lines.extend(
			[
				f"object {mesh.object_name} {{",
				f"mesh={mesh.name}",
				"position=(0,0,0)",
				"rotation=(0,0,0)",
				"scale=(1,1,1)",
				f"material={mesh.material_name}",
				"}",
			]
		)

	for light in lights:
		if light.kind == "area_light":
			lines.extend(
				[
					f"area_light {light.name} {{",
					f"position={fmt_vector(light.position)}",
					f"normal={fmt_vector(light.normal or Vector((0.0, -1.0, 0.0)))}",
					f"size=({fmt_float(light.width)},{fmt_float(light.height)})",
					f"color={fmt_color(light.color)}",
					f"intensity={fmt_float(light.intensity)}",
					"}",
				]
			)
		else:
			lines.extend(
				[
					f"point_light {light.name} {{",
					f"position={fmt_vector(light.position)}",
					f"radius={fmt_float(light.radius)}",
					f"color={fmt_color(light.color)}",
					f"intensity={fmt_float(light.intensity)}",
					"}",
				]
			)

	output_path.parent.mkdir(parents=True, exist_ok=True)
	output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
	args = parse_args(sys.argv)
	if args.blend:
		bpy.ops.wm.open_mainfile(filepath=args.blend)

	output_path = Path(args.output).expanduser().resolve()
	mesh_dir = Path(args.mesh_dir).expanduser()
	if not mesh_dir.is_absolute():
		mesh_dir = output_path.parent / mesh_dir

	materials: dict[str, LuzMaterial] = {}
	used_material_names: set[str] = set()
	bounds = Bounds()
	meshes = export_meshes(args, output_path, mesh_dir, materials, used_material_names, bounds)
	camera = export_camera(args, bounds)
	lights = export_lights(args, bounds)
	background_color = export_world_background()
	write_luz_scene(args, output_path, materials, meshes, camera, lights, background_color)

	triangle_count = sum(mesh.triangle_count for mesh in meshes)
	print(
		f"Exported {len(meshes)} mesh group(s), {triangle_count} triangle(s), "
		f"{len(materials)} material(s), and {len(lights)} light(s) to {output_path}"
	)


if __name__ == "__main__":
	main()
