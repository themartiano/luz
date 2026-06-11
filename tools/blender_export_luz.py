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
import time
from dataclasses import dataclass
from pathlib import Path

try:
	import bpy
	from mathutils import Vector
except ImportError as error:
	raise SystemExit("This exporter must be run inside Blender with --python.") from error


SAMPLE_TEXTURE_COLORS = True
TEXTURE_SAMPLE_SIZE = 64


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
	color: tuple[float, float, float]
	intensity: float
	position: Vector | None = None
	normal: Vector | None = None
	width: float = 1.0
	height: float = 1.0
	radius: float = 0.1
	atmosphere_angle: float | None = None


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
	parser.add_argument("--sun-power-scale", type=float, default=1.0, help="Multiplier from Blender sun energy to Luz intensity.")
	parser.add_argument("--camera-aperture", type=float, help="Override Luz camera lens diameter in world units.")
	parser.add_argument("--default-focus-distance", type=float, default=10.0, help="Fallback focus distance in Blender units.")
	parser.add_argument("--no-texture-colors", action="store_false", dest="sample_texture_colors", help="Skip image texture color approximation.")
	parser.add_argument("--texture-sample-size", type=int, default=64, help="Temporary texture thumbnail size used for image color averaging. Defaults to 64.")
	parser.add_argument("--profile", action="store_true", help="Print per-stage exporter progress and timings.")
	parser.set_defaults(sample_texture_colors=True)
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


def log_profile(args: argparse.Namespace, message: str) -> None:
	if args.profile:
		print(f"[luz-export] {message}", flush=True)


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


def color_has_energy(color: tuple[float, float, float]) -> bool:
	return max(color[0], color[1], color[2]) > 0.0


def average_image_pixels(image: object) -> tuple[float, float, float] | None:
	try:
		pixels = image.pixels[:]
		pixel_count = int(len(pixels) / 4)
	except (AttributeError, TypeError, ValueError, RuntimeError):
		return None
	if pixel_count <= 0:
		return None

	red = 0.0
	green = 0.0
	blue = 0.0
	for offset in range(0, pixel_count * 4, 4):
		red += float(pixels[offset])
		green += float(pixels[offset + 1])
		blue += float(pixels[offset + 2])
	return (red / pixel_count, green / pixel_count, blue / pixel_count)


def average_image_color(image: object | None) -> tuple[float, float, float] | None:
	if image is None:
		return None
	try:
		width = int(image.size[0])
		height = int(image.size[1])
	except (AttributeError, TypeError, ValueError):
		return None
	if width <= 0 or height <= 0:
		return None

	sample_size = max(int(TEXTURE_SAMPLE_SIZE), 1)
	if width <= sample_size and height <= sample_size:
		return average_image_pixels(image)

	scale = min(sample_size / width, sample_size / height)
	sample_width = max(int(width * scale), 1)
	sample_height = max(int(height * scale), 1)
	sampled_image = None
	try:
		sampled_image = image.copy()
		sampled_image.scale(sample_width, sample_height)
		return average_image_pixels(sampled_image)
	except Exception:
		return None
	finally:
		if sampled_image is not None:
			try:
				bpy.data.images.remove(sampled_image)
			except Exception:
				pass


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
	if node_type in {"TEX_IMAGE", "TEX_ENVIRONMENT"} and output_name == "Color":
		if not SAMPLE_TEXTURE_COLORS:
			return socket_default(output_socket, fallback)
		return average_image_color(getattr(node, "image", None)) or socket_default(output_socket, fallback)
	if node_type == "VALTORGB" and output_name == "Color":
		factor = scalar_from_value(resolve_socket_value(node_socket(node, "Fac") or node_socket(node, "Factor"), 0.5, visited, depth + 1), 0.5)
		try:
			return node.color_ramp.evaluate(max(0.0, min(1.0, factor)))
		except Exception:
			return socket_default(output_socket, fallback)
	if node_type in {"MIX_RGB", "MIX"} and output_name in {"Color", "Result"}:
		factor = scalar_from_value(resolve_socket_value(node_socket(node, "Fac") or node_socket(node, "Factor"), 0.5, visited, depth + 1), 0.5)
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
		if strength > 0.0 and color_has_energy(color):
			return (color, strength)
		return None

	if node.type == "BSDF_PRINCIPLED":
		color = color_from_value(
			socket_default_value(node, ("Emission Color", "Emission"), (1.0, 1.0, 1.0)),
			(1.0, 1.0, 1.0),
		)
		strength = scalar_from_value(socket_default_value(node, ("Emission Strength",), 0.0), 0.0)
		if strength > 0.0 and color_has_energy(color):
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
		if socket.name in {"Fac", "Factor"}:
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
			factor = scalar_from_value(socket_default_value(node, ("Fac", "Factor"), 0.5), 0.5)
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

	if not color_has_energy(emission_color):
		emission_strength = 0.0

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
ObjVectorKey = tuple[str, str, str]


def obj_vector_key(vector: Vector) -> ObjVectorKey:
	return (fmt_float(vector.x), fmt_float(vector.y), fmt_float(vector.z))


def obj_index_for(vector: Vector, registry: dict[ObjVectorKey, int], values: list[ObjVectorKey]) -> int:
	key = obj_vector_key(vector)
	index = registry.get(key)
	if index is not None:
		return index

	index = len(values) + 1
	registry[key] = index
	values.append(key)
	return index


def write_obj(mesh_path: Path, triangles: list[ObjTriangle]) -> None:
	vertex_registry: dict[ObjVectorKey, int] = {}
	normal_registry: dict[ObjVectorKey, int] = {}
	vertices: list[ObjVectorKey] = []
	normals: list[ObjVectorKey] = []
	faces: list[tuple[tuple[int, int], tuple[int, int], tuple[int, int]]] = []

	for triangle in triangles:
		face: list[tuple[int, int]] = []
		for vertex, normal in triangle:
			vertex_index = obj_index_for(vertex, vertex_registry, vertices)
			normal_index = obj_index_for(normal, normal_registry, normals)
			face.append((vertex_index, normal_index))
		faces.append((face[0], face[1], face[2]))

	mesh_path.parent.mkdir(parents=True, exist_ok=True)
	with mesh_path.open("w", encoding="utf-8") as stream:
		stream.write("# Generated by tools/blender_export_luz.py\n")
		for x, y, z in vertices:
			stream.write(f"v {x} {y} {z}\n")
		for x, y, z in normals:
			stream.write(f"vn {x} {y} {z}\n")
		for face in faces:
			stream.write(
				f"f {face[0][0]}//{face[0][1]} "
				f"{face[1][0]}//{face[1][1]} "
				f"{face[2][0]}//{face[2][1]}\n"
			)


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
	mesh_objects = [
		object_
		for object_ in bpy.context.scene.objects
		if object_.type == "MESH" and should_export_object(object_, args)
	]
	log_profile(args, f"Exporting {len(mesh_objects)} mesh object(s)")

	for object_index, object_ in enumerate(mesh_objects, start=1):
		object_start = time.perf_counter()
		log_profile(args, f"[{object_index}/{len(mesh_objects)}] Evaluating mesh object '{object_.name}'")
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
			luz_vertices: dict[int, Vector] = {}
			luz_normals: dict[int, Vector] = {}
			log_profile(
				args,
				f"[{object_index}/{len(mesh_objects)}] '{object_.name}' has "
				f"{len(mesh.vertices)} vertex/vertices and {len(mesh.loop_triangles)} triangle(s)",
			)

			def luz_vertex_for(vertex_index: int) -> Vector:
				luz_vertex = luz_vertices.get(vertex_index)
				if luz_vertex is None:
					blender_world = world_matrix @ mesh.vertices[vertex_index].co
					luz_vertex = blender_point_to_luz(blender_world, args.global_scale)
					luz_vertices[vertex_index] = luz_vertex
					bounds.include(luz_vertex)
				return luz_vertex

			def luz_normal_for(loop_index: int) -> Vector:
				luz_normal = luz_normals.get(loop_index)
				if luz_normal is None:
					luz_normal = loop_normal(mesh, loop_index, normal_matrix)
					luz_normals[loop_index] = luz_normal
				return luz_normal

			for loop_triangle in mesh.loop_triangles:
				material_index = loop_triangle.material_index
				triangle_corners: list[ObjCorner] = []
				for loop_index, vertex_index in zip(loop_triangle.loops, loop_triangle.vertices):
					luz_vertex = luz_vertex_for(vertex_index)
					luz_normal = luz_normal_for(loop_index)
					triangle_corners.append((luz_vertex, luz_normal))
				object_triangles_by_material.setdefault(material_index, []).append(
					(triangle_corners[0], triangle_corners[1], triangle_corners[2])
				)

			for material_index, triangles in object_triangles_by_material.items():
				if not triangles:
					continue

				material = material_for_slot(object_, material_index)
				material_label = getattr(material, "name_full", "default material") if material else "default material"
				material_start = time.perf_counter()
				log_profile(
					args,
					f"[{object_index}/{len(mesh_objects)}] Resolving material '{material_label}' "
					f"for {len(triangles)} triangle(s)",
				)
				material_name = export_material(material, used_material_names, materials)
				mesh_base = slugify(f"{object_.name}_{material_name}", "mesh")
				mesh_name = unique_name(mesh_base, used_mesh_names)
				object_name = unique_name(slugify(f"{object_.name}_{material_name}", "object"), used_object_names)
				mesh_path = mesh_dir / f"{mesh_name}.obj"
				scene_file_path = Path(os.path.relpath(mesh_path, output_path.parent)).as_posix()

				log_profile(args, f"[{object_index}/{len(mesh_objects)}] Writing {mesh_path.name}")
				write_obj(mesh_path, triangles)
				log_profile(
					args,
					f"[{object_index}/{len(mesh_objects)}] Wrote {mesh_path.name} "
					f"in {time.perf_counter() - material_start:.2f}s",
				)
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
		log_profile(args, f"[{object_index}/{len(mesh_objects)}] Finished '{object_.name}' in {time.perf_counter() - object_start:.2f}s")

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


def atmosphere_angle_from_sun_direction(direction: Vector) -> float | None:
	scene_to_sun = -direction
	projected = Vector((0.0, scene_to_sun.y, scene_to_sun.z))
	if projected.length == 0.0:
		return None
	projected.normalize()
	return math.atan2(-projected.z, projected.y) / math.pi


def export_lights(args: argparse.Namespace, bounds: Bounds) -> list[LuzLight]:
	lights: list[LuzLight] = []
	used_names: set[str] = set()
	center = bounds.center()
	light_objects = [
		object_
		for object_ in bpy.context.scene.objects
		if object_.type == "LIGHT" and should_export_object(object_, args)
	]
	log_profile(args, f"Exporting {len(light_objects)} light object(s)")

	for object_ in light_objects:
		data = object_.data
		name = unique_name(slugify(object_.name, "light"), used_names)
		position = blender_point_to_luz(object_.matrix_world.translation, args.global_scale)
		direction = blender_vector_to_luz(object_.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0)))
		color = light_color(data)
		energy = float(getattr(data, "energy", 1.0))
		intensity_scale = args.sun_power_scale if data.type == "SUN" else args.light_power_scale
		intensity = max(energy * intensity_scale, 0.0)
		log_profile(
			args,
			f"Light '{object_.name}' type={data.type} energy={fmt_float(energy)} "
			f"intensity={fmt_float(intensity)} direction={fmt_vector(direction)}",
		)

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
			lights.append(
				LuzLight(
					kind="directional_light",
					name=name,
					normal=direction,
					color=color,
					intensity=intensity,
					atmosphere_angle=atmosphere_angle_from_sun_direction(direction),
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


def mix_shader_uses_camera_ray_factor(node: object) -> bool:
	factor_socket = node_socket(node, "Fac") or node_socket(node, "Factor")
	try:
		links = factor_socket.links if factor_socket is not None else []
	except AttributeError:
		return False
	for link in links:
		from_node = getattr(link, "from_node", None)
		from_socket = getattr(link, "from_socket", None)
		if getattr(from_node, "type", "") == "LIGHT_PATH" and getattr(from_socket, "name", "") == "Is Camera Ray":
			return True
	return False


def multiply_color(color: tuple[float, float, float], strength: float) -> tuple[float, float, float]:
	return (
		max(color[0] * strength, 0.0),
		max(color[1] * strength, 0.0),
		max(color[2] * strength, 0.0),
	)


def add_color(first: tuple[float, float, float], second: tuple[float, float, float]) -> tuple[float, float, float]:
	return (
		max(first[0] + second[0], 0.0),
		max(first[1] + second[1], 0.0),
		max(first[2] + second[2], 0.0),
	)


def world_color_from_shader_node(
	node: object | None,
	default_color: tuple[float, float, float],
	camera_ray: bool,
	visited: set[object] | None = None,
) -> tuple[float, float, float]:
	if node is None:
		return default_color
	if visited is None:
		visited = set()
	if node in visited:
		return default_color
	visited.add(node)

	node_type = getattr(node, "type", "")
	if node_type == "BACKGROUND":
		color = color_from_value(socket_default_value(node, ("Color",), default_color), default_color)
		strength = scalar_from_value(socket_default_value(node, ("Strength",), 1.0), 1.0)
		return multiply_color(color, strength)

	if node_type == "EMISSION":
		color = color_from_value(socket_default_value(node, ("Color",), default_color), default_color)
		strength = scalar_from_value(socket_default_value(node, ("Strength",), 1.0), 1.0)
		return multiply_color(color, strength)

	if node_type == "MIX_SHADER":
		nodes = linked_shader_nodes(node)
		if len(nodes) >= 2:
			if mix_shader_uses_camera_ray_factor(node):
				branch = nodes[1] if camera_ray else nodes[0]
				return world_color_from_shader_node(branch, default_color, camera_ray, visited.copy())
			factor = scalar_from_value(socket_default_value(node, ("Fac", "Factor"), 0.5), 0.5)
			first = world_color_from_shader_node(nodes[0], default_color, camera_ray, visited.copy())
			second = world_color_from_shader_node(nodes[1], default_color, camera_ray, visited.copy())
			return blend_color(first, second, max(0.0, min(1.0, factor)))

	if node_type == "ADD_SHADER":
		nodes = linked_shader_nodes(node)
		if len(nodes) >= 2:
			first = world_color_from_shader_node(nodes[0], default_color, camera_ray, visited.copy())
			second = world_color_from_shader_node(nodes[1], default_color, camera_ray, visited.copy())
			return add_color(first, second)

	return default_color


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
	if background_node:
		return world_color_from_shader_node(background_node, default_color, True)
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


def exported_atmosphere_angle(lights: list[LuzLight]) -> float:
	for light in lights:
		if light.atmosphere_angle is not None:
			return light.atmosphere_angle
	return -0.4


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
		]
	)
	if sky == "atmosphere":
		lines.append(
			f"atmosphere={fmt_float(exported_atmosphere_angle(lights))},"
			"6360000,6420000,7994,1200,16,8,0.5"
		)
	lines.extend(
		[
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
						f"position={fmt_vector(light.position or Vector((0.0, 0.0, 0.0)))}",
					f"normal={fmt_vector(light.normal or Vector((0.0, -1.0, 0.0)))}",
					f"size=({fmt_float(light.width)},{fmt_float(light.height)})",
					f"color={fmt_color(light.color)}",
					f"intensity={fmt_float(light.intensity)}",
					"}",
				]
			)
		elif light.kind == "directional_light":
			lines.extend(
				[
					f"directional_light {light.name} {{",
					f"direction={fmt_vector(light.normal or Vector((0.0, -1.0, 0.0)))}",
					f"color={fmt_color(light.color)}",
					f"intensity={fmt_float(light.intensity)}",
					"}",
				]
			)
		else:
			lines.extend(
					[
						f"point_light {light.name} {{",
						f"position={fmt_vector(light.position or Vector((0.0, 0.0, 0.0)))}",
					f"radius={fmt_float(light.radius)}",
					f"color={fmt_color(light.color)}",
					f"intensity={fmt_float(light.intensity)}",
					"}",
				]
			)

	output_path.parent.mkdir(parents=True, exist_ok=True)
	output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
	global SAMPLE_TEXTURE_COLORS, TEXTURE_SAMPLE_SIZE
	start_time = time.perf_counter()
	args = parse_args(sys.argv)
	SAMPLE_TEXTURE_COLORS = args.sample_texture_colors
	TEXTURE_SAMPLE_SIZE = max(int(args.texture_sample_size), 1)
	if args.blend:
		log_profile(args, f"Opening blend file {args.blend}")
		bpy.ops.wm.open_mainfile(filepath=args.blend)

	output_path = Path(args.output).expanduser().resolve()
	mesh_dir = Path(args.mesh_dir).expanduser()
	if not mesh_dir.is_absolute():
		mesh_dir = output_path.parent / mesh_dir

	materials: dict[str, LuzMaterial] = {}
	used_material_names: set[str] = set()
	bounds = Bounds()
	log_profile(args, "Starting mesh export")
	meshes = export_meshes(args, output_path, mesh_dir, materials, used_material_names, bounds)
	log_profile(args, "Exporting camera")
	camera = export_camera(args, bounds)
	lights = export_lights(args, bounds)
	background_color = export_world_background()
	log_profile(args, f"World camera background={fmt_color(background_color)}")
	log_profile(args, f"Writing Luz scene {output_path}")
	write_luz_scene(args, output_path, materials, meshes, camera, lights, background_color)

	triangle_count = sum(mesh.triangle_count for mesh in meshes)
	print(
		f"Exported {len(meshes)} mesh group(s), {triangle_count} triangle(s), "
		f"{len(materials)} material(s), and {len(lights)} light(s) to {output_path} "
		f"in {time.perf_counter() - start_time:.2f}s"
	)


if __name__ == "__main__":
	main()
