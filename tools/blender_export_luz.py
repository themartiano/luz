#!/usr/bin/env python3
"""
Export the current Blender scene to a Luz .luz file plus simple OBJ meshes.

Run from Blender:
  blender -b scene.blend --python tools/blender_export_luz.py -- --output exports/scene.luz
"""

from __future__ import annotations

import argparse
from array import array
from collections.abc import Iterable
import math
import os
import re
import shutil
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

try:
	import bpy
	from mathutils import Vector
except ImportError as error:
	bpy = None
	_BLENDER_IMPORT_ERROR = error

	class Vector:  # type: ignore[no-redef]
		def __init__(self, *_args: object, **_kwargs: object) -> None:
			raise RuntimeError("mathutils.Vector is only available inside Blender.")


SAMPLE_TEXTURE_COLORS = True
TEXTURE_SAMPLE_SIZE = 64
DEFAULT_QUALITY_MODE = "on"
DEFAULT_VIEW_TRANSFORM_MODE = "auto"
IMAGE_AVERAGE_CACHE: dict[tuple[int, int, int, int], tuple[float, float, float] | None] = {}


@dataclass
class LuzMaterial:
	name: str
	material_type: str
	base_color: tuple[float, float, float]
	metallic: float
	roughness: float
	alpha: float
	transmission: float
	refractive_index: float
	clearcoat: float
	clearcoat_roughness: float
	sheen: float
	subsurface: float
	subsurface_radius: tuple[float, float, float]
	subsurface_scale: float
	subsurface_color: tuple[float, float, float]
	subsurface_method: str
	glossy_color: tuple[float, float, float]
	glossy_weight: float
	emission_color: tuple[float, float, float]
	emission_strength: float
	texture_path: str | None = None


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
	focal_length_mm: float
	sensor_width_mm: float
	sensor_height_mm: float
	f_stop: float
	pinhole: bool
	focus_distance_meters: float


@dataclass
class LuzLight:
	kind: str
	name: str
	color: tuple[float, float, float]
	quantity: float
	unit_key: str
	position: Vector | None = None
	normal: Vector | None = None
	width: float = 1.0
	height: float = 1.0
	radius: float = 0.1
	atmosphere_angle: float | None = None


@dataclass
class LuzEnvironment:
	path: str
	strength: float
	rotation: float = 0.0


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
	parser.add_argument("--texture-dir", default="textures", help="Texture output directory, relative to the .luz file by default.")
	parser.add_argument("--selected-only", action="store_true", help="Export selected objects only.")
	parser.add_argument("--include-hidden", action="store_true", help="Include objects hidden from render.")
	parser.add_argument("--resolution", help="Override resolution as WIDTHxHEIGHT.")
	parser.add_argument("--samples", type=int, help="Override Luz samples per pixel.")
	parser.add_argument("--adaptive", choices=("auto", "on", "off", "true", "false", "1", "0"), default=DEFAULT_QUALITY_MODE, help="Enable Luz adaptive sampling by default; use auto to mirror Blender or off to disable.")
	parser.add_argument("--adaptive-min-samples", type=int, help="Override exported Luz adaptive minimum samples.")
	parser.add_argument("--adaptive-threshold", type=float, help="Override exported Luz adaptive noise threshold.")
	parser.add_argument("--adaptive-check-interval", type=int, help="Override exported Luz adaptive check interval.")
	parser.add_argument("--max-light-bounces", type=int, help="Override Luz max light bounces.")
	parser.add_argument("--denoise", choices=("auto", "on", "off", "true", "false", "1", "0"), default=DEFAULT_QUALITY_MODE, help="Enable Luz denoising by default; use auto to mirror Blender or off to disable.")
	parser.add_argument("--view-transform", choices=("auto", "standard", "agx", "aces", "raw"), default=DEFAULT_VIEW_TRANSFORM_MODE, help="Luz view transform. Raw is for debugging/HDR data, not display viewing.")
	parser.add_argument("--exposure", type=float, help="Override exported Luz exposure compensation in stops.")
	parser.add_argument("--sky", choices=("linear", "none", "atmosphere", "environment"), help="Override Luz sky mode.")
	parser.add_argument("--render-output", help="Luz render output filename. .bmp is appended by Luz when omitted.")
	parser.add_argument("--global-scale", type=float, default=1.0, help="Scale all exported positions and mesh vertices.")
	parser.add_argument("--light-power-scale", type=float, default=1.0, help="Additional multiplier for exported area and point light power.")
	parser.add_argument("--subsurface-scale-multiplier", type=float, default=1.0, help="Additional multiplier for exported Principled subsurface scale.")
	parser.add_argument("--min-point-light-radius", type=float, default=0.1, help="Minimum Blender-unit radius for point and spot lights. Defaults to 0.1.")
	parser.add_argument("--sun-power-scale", type=float, default=1.0, help="Multiplier from Blender sun energy to Luz directional irradiance.")
	parser.add_argument("--camera-f-stop", type=float, help="Override exported camera f-number.")
	parser.add_argument("--default-focus-distance", type=float, default=10.0, help="Fallback focus distance in Blender units.")
	parser.add_argument("--no-texture-colors", action="store_false", dest="sample_texture_colors", help="Skip image texture color approximation.")
	parser.add_argument("--texture-sample-size", type=int, default=64, help="Temporary texture thumbnail size used for image color averaging. Defaults to 64.")
	parser.add_argument("--texture-max-size", type=int, default=1024, help="Maximum exported texture side length. Defaults to 1024.")
	parser.add_argument("--profile", action="store_true", help="Print per-stage exporter progress and timings.")
	parser.set_defaults(sample_texture_colors=True)
	args = parser.parse_args(argv)
	if not math.isfinite(args.global_scale) or args.global_scale <= 0.0:
		parser.error("--global-scale must be a finite positive value.")
	if not math.isfinite(args.subsurface_scale_multiplier) or args.subsurface_scale_multiplier <= 0.0:
		parser.error("--subsurface-scale-multiplier must be a finite positive value.")
	if not math.isfinite(args.min_point_light_radius) or args.min_point_light_radius < 0.0:
		parser.error("--min-point-light-radius must be a finite non-negative value.")
	if args.camera_f_stop is not None and (not math.isfinite(args.camera_f_stop) or args.camera_f_stop <= 0.0):
		parser.error("--camera-f-stop must be a finite positive value.")
	if args.exposure is not None and not math.isfinite(args.exposure):
		parser.error("--exposure must be finite.")
	if args.adaptive_min_samples is not None and args.adaptive_min_samples <= 0:
		parser.error("--adaptive-min-samples must be positive.")
	if args.adaptive_threshold is not None and (not math.isfinite(args.adaptive_threshold) or args.adaptive_threshold <= 0.0):
		parser.error("--adaptive-threshold must be a finite positive value.")
	if args.adaptive_check_interval is not None and args.adaptive_check_interval <= 0:
		parser.error("--adaptive-check-interval must be positive.")
	return args


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


def fmt_blender_color(value: tuple[float, float, float]) -> str:
	return f"linear_srgb{fmt_color(value)}"


def log_profile(args: argparse.Namespace, message: str) -> None:
	if args.profile:
		print(f"[luz-export] {message}", flush=True)


def blender_point_to_luz(point: Vector, global_scale: float) -> Vector:
	return Vector((point.x * global_scale, point.z * global_scale, -point.y * global_scale))


def blender_meters_per_unit() -> float:
	try:
		scale_length = float(bpy.context.scene.unit_settings.scale_length)
	except (AttributeError, TypeError, ValueError):
		return 1.0
	if not math.isfinite(scale_length) or scale_length <= 0.0:
		return 1.0
	return scale_length


def luz_meters_per_unit(args: argparse.Namespace) -> float:
	return blender_meters_per_unit() / args.global_scale


def blender_vector_to_luz(vector: Vector) -> Vector:
	result = Vector((vector.x, vector.z, -vector.y))
	if result.length == 0.0:
		return Vector((0.0, 0.0, -1.0))
	return result.normalized()


def blender_normal_to_luz(normal: Vector) -> Vector:
	return blender_vector_to_luz(normal)


def effective_camera_sensor_mm(
	camera_data: object,
	scene: object,
	focal_length_mm: float,
	fallback_width_mm: float,
	fallback_height_mm: float,
) -> tuple[float, float]:
	if getattr(camera_data, "type", "PERSP") != "PERSP":
		return (fallback_width_mm, fallback_height_mm)
	try:
		frame = camera_data.view_frame(scene=scene)
	except Exception:
		return (fallback_width_mm, fallback_height_mm)
	if len(frame) < 4:
		return (fallback_width_mm, fallback_height_mm)

	try:
		min_x = min(float(point.x) for point in frame)
		max_x = max(float(point.x) for point in frame)
		min_y = min(float(point.y) for point in frame)
		max_y = max(float(point.y) for point in frame)
		depth = sum(abs(float(point.z)) for point in frame) / len(frame)
	except (AttributeError, TypeError, ValueError):
		return (fallback_width_mm, fallback_height_mm)

	frame_width = max_x - min_x
	frame_height = max_y - min_y
	if (
		not math.isfinite(frame_width)
		or not math.isfinite(frame_height)
		or not math.isfinite(depth)
		or frame_width <= 0.0
		or frame_height <= 0.0
		or depth <= 1e-12
	):
		return (fallback_width_mm, fallback_height_mm)

	return (
		max(frame_width * focal_length_mm / depth, 1e-6),
		max(frame_height * focal_length_mm / depth, 1e-6),
	)


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


def channel_index_from_name(name: str) -> int | None:
	channel = name.strip().lower()
	if channel in {"red", "r", "x"}:
		return 0
	if channel in {"green", "g", "y"}:
		return 1
	if channel in {"blue", "b", "z"}:
		return 2
	if channel in {"alpha", "a"}:
		return 3
	return None


def channel_from_value(value: object, channel_index: int, fallback: float) -> float:
	try:
		return float(value[channel_index])  # type: ignore[index]
	except (TypeError, IndexError, ValueError):
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


def image_dimensions(image: object) -> tuple[int, int] | None:
	try:
		width = int(image.size[0])
		height = int(image.size[1])
	except (AttributeError, TypeError, ValueError):
		return None
	if width <= 0 or height <= 0:
		return None
	return (width, height)


def image_pixels(image: object, width: int, height: int) -> object | None:
	expected_values = width * height * 4
	if expected_values <= 0:
		return None
	try:
		pixels = image.pixels
		foreach_get = getattr(pixels, "foreach_get", None)
		if foreach_get is not None:
			buffer = array("f", [0.0]) * expected_values
			foreach_get(buffer)
			return buffer
		values = pixels[:]
	except (AttributeError, TypeError, ValueError, RuntimeError):
		return None
	try:
		if len(values) < expected_values:
			return None
	except TypeError:
		return None
	return values


def remove_blender_image(image: object) -> None:
	try:
		bpy.data.images.remove(image)
	except Exception:
		pass


def average_image_pixels(image: object) -> tuple[float, float, float] | None:
	dimensions = image_dimensions(image)
	if dimensions is None:
		return None
	width, height = dimensions
	pixels = image_pixels(image, width, height)
	if pixels is None:
		return None
	pixel_count = width * height
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
	dimensions = image_dimensions(image)
	if dimensions is None:
		return None
	width, height = dimensions

	sample_size = max(int(TEXTURE_SAMPLE_SIZE), 1)
	cache_key = (id(image), width, height, sample_size)
	if cache_key in IMAGE_AVERAGE_CACHE:
		return IMAGE_AVERAGE_CACHE[cache_key]

	if width <= sample_size and height <= sample_size:
		result = average_image_pixels(image)
		IMAGE_AVERAGE_CACHE[cache_key] = result
		return result

	scale = min(sample_size / width, sample_size / height)
	sample_width = max(int(width * scale), 1)
	sample_height = max(int(height * scale), 1)
	sampled_image = None
	try:
		sampled_image = image.copy()
		sampled_image.scale(sample_width, sample_height)
		result = average_image_pixels(sampled_image)
	except Exception:
		result = None
	finally:
		if sampled_image is not None:
			remove_blender_image(sampled_image)
	IMAGE_AVERAGE_CACHE[cache_key] = result
	return result


def image_copy_scaled(image: object, max_size: int) -> object | None:
	dimensions = image_dimensions(image)
	if dimensions is None:
		return None
	width, height = dimensions

	max_size = max(int(max_size), 1)
	if width <= max_size and height <= max_size:
		return image

	scale = min(max_size / width, max_size / height)
	sample_width = max(int(width * scale), 1)
	sample_height = max(int(height * scale), 1)
	try:
		sampled_image = image.copy()
		sampled_image.scale(sample_width, sample_height)
		return sampled_image
	except Exception:
		return None


def channel_byte(value: float) -> int:
	try:
		channel = float(value)
	except (TypeError, ValueError):
		return 0
	if math.isnan(channel) or channel <= 0.0:
		return 0
	if channel >= 1.0:
		return 255
	return int(channel * 255.0 + 0.5)


def ppm_rgb_bytes(pixels: object, width: int, height: int) -> bytearray:
	output = bytearray(width * height * 3)
	output_offset = 0
	to_byte = channel_byte
	for y in range(height - 1, -1, -1):
		pixel_offset = y * width * 4
		row_end = pixel_offset + width * 4
		while pixel_offset < row_end:
			output[output_offset] = to_byte(pixels[pixel_offset])  # type: ignore[index]
			output[output_offset + 1] = to_byte(pixels[pixel_offset + 1])  # type: ignore[index]
			output[output_offset + 2] = to_byte(pixels[pixel_offset + 2])  # type: ignore[index]
			output_offset += 3
			pixel_offset += 4
	return output


def write_image_as_ppm(image: object, texture_path: Path, max_size: int) -> bool:
	sampled_image = image_copy_scaled(image, max_size)
	if sampled_image is None:
		return False

	remove_sampled_image = sampled_image is not image
	dimensions = image_dimensions(sampled_image)
	if dimensions is None:
		if remove_sampled_image:
			remove_blender_image(sampled_image)
		return False
	width, height = dimensions
	pixels = image_pixels(sampled_image, width, height)
	if pixels is None:
		if remove_sampled_image:
			remove_blender_image(sampled_image)
		return False

	try:
		texture_path.parent.mkdir(parents=True, exist_ok=True)
		with texture_path.open("wb") as stream:
			stream.write(f"P6\n{width} {height}\n255\n".encode("ascii"))
			stream.write(ppm_rgb_bytes(pixels, width, height))
	except (IndexError, OSError, TypeError, ValueError):
		return False
	finally:
		if remove_sampled_image:
			remove_blender_image(sampled_image)

	return True


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
	if node_type in {"SEPARATE_COLOR", "SEPRGB"}:
		channel_index = channel_index_from_name(output_name)
		if channel_index is None:
			return socket_default(output_socket, fallback)
		color = resolve_socket_value(
			node_socket(node, "Color") or node_socket(node, "Image"),
			fallback,
			visited,
			depth + 1,
		)
		return channel_from_value(
			color,
			channel_index,
			scalar_from_value(socket_default(output_socket, fallback), scalar_from_value(fallback, 0.0)),
		)
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


def texture_image_from_output(output_socket: object, visited: set[object], depth: int = 0) -> object | None:
	node = getattr(output_socket, "node", None)
	if node is None or node in visited or depth > 8:
		return None
	visited.add(node)

	output_name = getattr(output_socket, "name", "")
	node_type = getattr(node, "type", "")
	if node_type in {"TEX_IMAGE", "TEX_ENVIRONMENT"} and output_name == "Color":
		return getattr(node, "image", None)
	if node_type in {"MIX_RGB", "MIX"} and output_name in {"Color", "Result"}:
		for input_name in ("Color1", "Color2", "A", "B"):
			socket = node_socket(node, input_name)
			if socket is None:
				continue
			for link in getattr(socket, "links", []):
				image = texture_image_from_output(link.from_socket, visited.copy(), depth + 1)
				if image is not None:
					return image
		return None
	if node_type == "VALTORGB" and output_name == "Color":
		socket = node_socket(node, "Fac") or node_socket(node, "Factor")
		for link in getattr(socket, "links", []) if socket is not None else []:
			image = texture_image_from_output(link.from_socket, visited.copy(), depth + 1)
			if image is not None:
				return image
		return None
	if node_type in {"RGB", "VALUE"}:
		return None
	for socket in getattr(node, "inputs", []):
		for link in getattr(socket, "links", []):
			image = texture_image_from_output(link.from_socket, visited.copy(), depth + 1)
			if image is not None:
				return image
	return None


def texture_image_from_socket(socket: object | None) -> object | None:
	if socket is None:
		return None
	try:
		if not socket.links:
			return None
	except AttributeError:
		return None
	return texture_image_from_output(socket.links[0].from_socket, set())


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

	def surface_node_from_output(output_node: object) -> object | None:
		surface = output_node.inputs.get("Surface")
		if surface and surface.links:
			return surface.links[0].from_node
		return None

	output_nodes: list[object] = []
	for node in material.node_tree.nodes:
		if node.type != "OUTPUT_MATERIAL":
			continue
		if surface_node_from_output(node) is not None:
			output_nodes.append(node)
	if not output_nodes:
		return None

	try:
		render_engine = str(bpy.context.scene.render.engine).upper()
	except AttributeError:
		render_engine = ""
	target = "CYCLES" if render_engine == "CYCLES" else "EEVEE" if "EEVEE" in render_engine else "ALL"

	def node_target(node: object) -> str:
		return str(getattr(node, "target", "ALL") or "ALL").upper()

	def active_output(node: object) -> bool:
		return bool(getattr(node, "is_active_output", False))

	preference_groups = []
	if target != "ALL":
		preference_groups.append(lambda candidate: node_target(candidate) == target)
	preference_groups.extend(
		[
			lambda candidate: node_target(candidate) == "ALL" and active_output(candidate),
			lambda candidate: node_target(candidate) == "ALL",
			active_output,
			lambda candidate: True,
		]
	)

	for matches in preference_groups:
		for node in output_nodes:
			if matches(node):
				return surface_node_from_output(node)
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
	refractive_index: float,
	clearcoat: float,
	clearcoat_roughness: float,
	sheen: float,
	subsurface: float,
	subsurface_radius: tuple[float, float, float],
	subsurface_scale: float,
	subsurface_color: tuple[float, float, float],
	subsurface_method: str,
	glossy_color: tuple[float, float, float],
	glossy_weight: float,
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
		"refractive_index": refractive_index,
		"clearcoat": clearcoat,
		"clearcoat_roughness": clearcoat_roughness,
		"sheen": sheen,
		"subsurface": subsurface,
		"subsurface_radius": subsurface_radius,
		"subsurface_scale": subsurface_scale,
		"subsurface_color": subsurface_color,
		"subsurface_method": subsurface_method,
		"glossy_color": glossy_color,
		"glossy_weight": glossy_weight,
		"emission_color": emission_color,
		"emission_strength": emission_strength,
	}


def export_texture_image(
	image: object,
	material_name: str,
	texture_dir: Path,
	scene_texture_dir: Path,
	used_texture_names: set[str],
	exported_textures: dict[str, str],
	args: argparse.Namespace,
) -> str | None:
	image_key = getattr(image, "name_full", None) or getattr(image, "name", None)
	if image_key and image_key in exported_textures:
		return exported_textures[image_key]

	base_name = slugify(getattr(image, "name", material_name), "texture")
	texture_name = unique_name(base_name, used_texture_names) + ".ppm"
	texture_path = texture_dir / texture_name
	scene_path = (scene_texture_dir / texture_name).as_posix()

	if not write_image_as_ppm(image, texture_path, int(args.texture_max_size)):
		return None
	if image_key:
		exported_textures[image_key] = scene_path
	return scene_path


def image_source_path(image: object) -> Path | None:
	raw_path = getattr(image, "filepath", None) or getattr(image, "filepath_raw", None)
	if not raw_path:
		return None
	try:
		absolute_path = Path(bpy.path.abspath(raw_path)).expanduser()
	except Exception:
		return None
	if not absolute_path.exists() or not absolute_path.is_file():
		return None
	return absolute_path


def export_environment_image(
	image: object,
	texture_dir: Path,
	scene_texture_dir: Path,
	used_texture_names: set[str],
	exported_textures: dict[str, str],
	args: argparse.Namespace,
) -> str | None:
	image_key = "environment::" + str(getattr(image, "name_full", None) or getattr(image, "name", "environment"))
	if image_key in exported_textures:
		return exported_textures[image_key]

	source_path = image_source_path(image)
	if source_path is not None and source_path.suffix.lower() in {".hdr", ".pic"}:
		base_name = slugify(source_path.stem, "environment")
		texture_name = unique_name(base_name, used_texture_names) + source_path.suffix.lower()
		texture_path = texture_dir / texture_name
		scene_path = (scene_texture_dir / texture_name).as_posix()
		texture_path.parent.mkdir(parents=True, exist_ok=True)
		try:
			shutil.copyfile(source_path, texture_path)
		except OSError:
			return None
		exported_textures[image_key] = scene_path
		return scene_path

	base_name = slugify(getattr(image, "name", "environment"), "environment")
	texture_name = unique_name(base_name, used_texture_names) + ".ppm"
	texture_path = texture_dir / texture_name
	scene_path = (scene_texture_dir / texture_name).as_posix()
	if not write_image_as_ppm(image, texture_path, int(args.texture_max_size)):
		return None
	exported_textures[image_key] = scene_path
	return scene_path


def blend_material_values(
	first: dict[str, object],
	second: dict[str, object],
	factor: float,
) -> dict[str, object]:
	first_type = str(first["material_type"])
	second_type = str(second["material_type"])
	if {first_type, second_type} == {"lambertian", "glossy"}:
		diffuse = first if first_type == "lambertian" else second
		glossy = first if first_type == "glossy" else second
		glossy_weight = 1.0 - factor if first_type == "glossy" else factor
		return {
			"material_type": "diffuse_glossy",
			"base_color": diffuse["base_color"],
			"metallic": 0.0,
			"roughness": glossy["roughness"],
			"alpha": blend_scalar(float(first["alpha"]), float(second["alpha"]), factor),
			"transmission": 0.0,
			"refractive_index": blend_scalar(float(first["refractive_index"]), float(second["refractive_index"]), factor),
			"clearcoat": 0.0,
			"clearcoat_roughness": glossy["roughness"],
			"sheen": 0.0,
			"subsurface": 0.0,
			"subsurface_radius": (1.0, 1.0, 1.0),
			"subsurface_scale": 0.001,
			"subsurface_color": (1.0, 1.0, 1.0),
			"subsurface_method": "burley",
			"glossy_color": glossy["glossy_color"],
			"glossy_weight": max(0.0, min(1.0, glossy_weight)),
			"emission_color": blend_color(first["emission_color"], second["emission_color"], factor),  # type: ignore[arg-type]
			"emission_strength": blend_scalar(float(first["emission_strength"]), float(second["emission_strength"]), factor),
		}
	return {
		"material_type": first["material_type"] if first_type == second_type else "principled",
		"base_color": blend_color(first["base_color"], second["base_color"], factor),  # type: ignore[arg-type]
		"metallic": blend_scalar(float(first["metallic"]), float(second["metallic"]), factor),
		"roughness": blend_scalar(float(first["roughness"]), float(second["roughness"]), factor),
		"alpha": blend_scalar(float(first["alpha"]), float(second["alpha"]), factor),
		"transmission": blend_scalar(float(first["transmission"]), float(second["transmission"]), factor),
		"refractive_index": blend_scalar(float(first["refractive_index"]), float(second["refractive_index"]), factor),
		"clearcoat": blend_scalar(float(first["clearcoat"]), float(second["clearcoat"]), factor),
		"clearcoat_roughness": blend_scalar(float(first["clearcoat_roughness"]), float(second["clearcoat_roughness"]), factor),
		"sheen": blend_scalar(float(first["sheen"]), float(second["sheen"]), factor),
		"subsurface": blend_scalar(float(first["subsurface"]), float(second["subsurface"]), factor),
		"subsurface_radius": blend_color(first["subsurface_radius"], second["subsurface_radius"], factor),  # type: ignore[arg-type]
		"subsurface_scale": blend_scalar(float(first["subsurface_scale"]), float(second["subsurface_scale"]), factor),
		"subsurface_color": blend_color(first["subsurface_color"], second["subsurface_color"], factor),  # type: ignore[arg-type]
		"subsurface_method": first["subsurface_method"] if str(first["subsurface_method"]) == str(second["subsurface_method"]) else "burley",
		"glossy_color": blend_color(first["glossy_color"], second["glossy_color"], factor),  # type: ignore[arg-type]
		"glossy_weight": blend_scalar(float(first["glossy_weight"]), float(second["glossy_weight"]), factor),
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
		values["refractive_index"] = scalar_from_value(
			socket_default_value(node, ("IOR",), values["refractive_index"]),
			float(values["refractive_index"]),
		)
		values["clearcoat"] = scalar_from_value(
			socket_default_value(node, ("Coat Weight", "Clearcoat", "Clearcoat Weight", "Clear Coat Weight", "Coat"), values["clearcoat"]),
			float(values["clearcoat"]),
		)
		values["clearcoat_roughness"] = scalar_from_value(
			socket_default_value(node, ("Coat Roughness", "Clearcoat Roughness", "Clear Coat Roughness"), values["clearcoat_roughness"]),
			float(values["clearcoat_roughness"]),
		)
		values["sheen"] = scalar_from_value(
			socket_default_value(node, ("Sheen Weight", "Sheen"), values["sheen"]),
			float(values["sheen"]),
		)
		values["subsurface"] = scalar_from_value(
			socket_default_value(node, ("Subsurface Weight", "Subsurface"), values["subsurface"]),
			float(values["subsurface"]),
		)
		values["subsurface_radius"] = color_from_value(
			socket_default_value(node, ("Subsurface Radius",), values["subsurface_radius"]),
			values["subsurface_radius"],  # type: ignore[arg-type]
		)
		values["subsurface_scale"] = scalar_from_value(
			socket_default_value(node, ("Subsurface Scale",), values["subsurface_scale"]),
			float(values["subsurface_scale"]),
		)
		values["subsurface_color"] = color_from_value(
			socket_default_value(node, ("Subsurface Color",), values["subsurface_color"]),
			values["subsurface_color"],  # type: ignore[arg-type]
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
		values["material_type"] = "glossy"
		values["base_color"] = color_from_value(socket_default_value(node, ("Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["glossy_color"] = values["base_color"]
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), values["roughness"]), float(values["roughness"]))
		values["metallic"] = 0.0
		values["clearcoat"] = 0.0
		values["clearcoat_roughness"] = values["roughness"]
		values["glossy_weight"] = 1.0
		return values

	if node_type in {"BSDF_GLASS", "BSDF_TRANSLUCENT", "BSDF_TRANSPARENT"}:
		values["material_type"] = "dielectric"
		values["base_color"] = color_from_value(socket_default_value(node, ("Color",), values["base_color"]), values["base_color"])  # type: ignore[arg-type]
		values["roughness"] = scalar_from_value(socket_default_value(node, ("Roughness",), values["roughness"]), float(values["roughness"]))
		values["refractive_index"] = scalar_from_value(socket_default_value(node, ("IOR",), values["refractive_index"]), float(values["refractive_index"]))
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


def base_color_texture_from_shader_node(
	node: object | None,
	visited: set[object] | None = None,
) -> object | None:
	if node is None:
		return None
	if visited is None:
		visited = set()
	if node in visited:
		return None
	visited.add(node)

	node_type = getattr(node, "type", "")
	if node_type == "BSDF_PRINCIPLED":
		return texture_image_from_socket(node_socket(node, "Base Color"))
	if node_type in {"BSDF_DIFFUSE", "BSDF_GLOSSY", "BSDF_ANISOTROPIC"}:
		return texture_image_from_socket(node_socket(node, "Color"))

	for child in linked_shader_nodes(node):
		image = base_color_texture_from_shader_node(child, visited.copy())
		if image is not None:
			return image
	return None


def export_material(
	material: object | None,
	used_names: set[str],
	registry: dict[str, LuzMaterial],
	texture_dir: Path,
	scene_texture_dir: Path,
	used_texture_names: set[str],
	exported_textures: dict[str, str],
	args: argparse.Namespace,
) -> str:
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
	refractive_index = 1.5
	clearcoat = 0.0
	clearcoat_roughness = 0.03
	sheen = 0.0
	subsurface = 0.0
	subsurface_radius = (1.0, 1.0, 1.0)
	subsurface_scale = 0.001
	subsurface_color = (1.0, 1.0, 1.0)
	subsurface_method = "burley"
	glossy_color = (1.0, 1.0, 1.0)
	glossy_weight = 0.0
	emission_color = (1.0, 1.0, 1.0)
	emission_strength = 0.0
	texture_path: str | None = None

	if material:
		base_color = color_from_value(getattr(material, "diffuse_color", base_color), base_color)
		try:
			alpha = float(material.diffuse_color[3])
		except (AttributeError, IndexError, TypeError, ValueError):
			alpha = 1.0

		surface_node = material_output_surface_node(material)
		shader_node = surface_node or find_principled_node(material)
		if shader_node:
			values = material_values_from_shader_node(
				shader_node,
				material_defaults(
					material_type,
					base_color,
					metallic,
					roughness,
					alpha,
					transmission,
					refractive_index,
					clearcoat,
					clearcoat_roughness,
					sheen,
					subsurface,
					subsurface_radius,
					subsurface_scale,
					subsurface_color,
					subsurface_method,
					glossy_color,
					glossy_weight,
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
			refractive_index = float(values["refractive_index"])
			clearcoat = float(values["clearcoat"])
			clearcoat_roughness = float(values["clearcoat_roughness"])
			sheen = float(values["sheen"])
			subsurface = float(values["subsurface"])
			subsurface_radius = values["subsurface_radius"]  # type: ignore[assignment]
			subsurface_scale = float(values["subsurface_scale"])
			subsurface_color = values["subsurface_color"]  # type: ignore[assignment]
			subsurface_method = str(values["subsurface_method"])
			glossy_color = values["glossy_color"]  # type: ignore[assignment]
			glossy_weight = float(values["glossy_weight"])
			emission_color = values["emission_color"]  # type: ignore[assignment]
			emission_strength = float(values["emission_strength"])

			base_color_texture = base_color_texture_from_shader_node(shader_node)
			if base_color_texture is not None and SAMPLE_TEXTURE_COLORS:
				texture_path = export_texture_image(
					base_color_texture,
					name,
					texture_dir,
					scene_texture_dir,
					used_texture_names,
					exported_textures,
					args,
				)
				if texture_path is not None:
					base_color = (1.0, 1.0, 1.0)

		emission = material_emission(material)
		if emission:
			emission_color = emission[0]
			emission_strength = emission[1]

	if texture_path is not None:
		base_color = (1.0, 1.0, 1.0)
	if subsurface > 0.0:
		subsurface_scale *= args.subsurface_scale_multiplier
	if not color_has_energy(emission_color):
		emission_strength = 0.0
	if emission_strength > 0.0:
		material_type = "emissive"
		base_color = emission_color

	registry[key] = LuzMaterial(
		name=name,
		material_type=material_type,
		base_color=base_color,
		metallic=metallic,
		roughness=roughness,
		alpha=alpha,
		transmission=transmission,
		refractive_index=refractive_index,
		clearcoat=clearcoat,
		clearcoat_roughness=clearcoat_roughness,
		sheen=sheen,
		subsurface=subsurface,
		subsurface_radius=subsurface_radius,
		subsurface_scale=subsurface_scale,
		subsurface_color=subsurface_color,
		subsurface_method=subsurface_method,
		glossy_color=glossy_color,
		glossy_weight=glossy_weight,
		emission_color=emission_color,
		emission_strength=emission_strength,
		texture_path=texture_path,
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


def loop_normal_from_data(corner_normals: object | None, loops: object, loop_index: int, normal_matrix: object) -> Vector:
	try:
		if corner_normals is not None:
			blender_normal = corner_normals[loop_index].vector.copy()  # type: ignore[index]
		else:
			blender_normal = loops[loop_index].normal.copy()  # type: ignore[index]
	except (AttributeError, IndexError):
		blender_normal = loops[loop_index].normal.copy()  # type: ignore[index]

	world_normal = normal_matrix @ blender_normal
	return blender_normal_to_luz(world_normal)


def loop_normal(mesh: object, loop_index: int, normal_matrix: object) -> Vector:
	return loop_normal_from_data(getattr(mesh, "corner_normals", None), mesh.loops, loop_index, normal_matrix)


def active_uv_layer_data(mesh: object) -> object | None:
	try:
		uv_layer = mesh.uv_layers.active
		if uv_layer is None:
			return None
		return uv_layer.data
	except AttributeError:
		return None


def loop_uv_from_data(uv_layer_data: object | None, loop_index: int) -> tuple[float, float]:
	if uv_layer_data is None:
		return (0.0, 0.0)
	try:
		uv = uv_layer_data[loop_index].uv  # type: ignore[index]
		return (float(uv.x), float(uv.y))
	except (AttributeError, IndexError, TypeError, ValueError):
		return (0.0, 0.0)


def loop_uv(mesh: object, loop_index: int) -> tuple[float, float]:
	return loop_uv_from_data(active_uv_layer_data(mesh), loop_index)


ObjUVKey = tuple[str, str]
ObjVectorKey = tuple[str, str, str]
ObjFaceCorner = tuple[int, int, int]
ObjFace = tuple[ObjFaceCorner, ObjFaceCorner, ObjFaceCorner]
OBJ_WRITE_CHUNK_LINES = 8192


def obj_vector_key(vector: Vector) -> ObjVectorKey:
	return (fmt_float(vector.x), fmt_float(vector.y), fmt_float(vector.z))


def obj_index_for_key(key: ObjVectorKey, registry: dict[ObjVectorKey, int], values: list[ObjVectorKey]) -> int:
	index = registry.get(key)
	if index is not None:
		return index

	index = len(values) + 1
	registry[key] = index
	values.append(key)
	return index


def obj_uv_key(uv: tuple[float, float]) -> ObjUVKey:
	return (fmt_float(uv[0]), fmt_float(uv[1]))


def obj_uv_index_for_key(key: ObjUVKey, registry: dict[ObjUVKey, int], values: list[ObjUVKey]) -> int:
	index = registry.get(key)
	if index is not None:
		return index

	index = len(values) + 1
	registry[key] = index
	values.append(key)
	return index


def write_text_line_chunks(stream: TextIO, lines: Iterable[str]) -> None:
	chunk: list[str] = []
	for line in lines:
		chunk.append(line)
		if len(chunk) >= OBJ_WRITE_CHUNK_LINES:
			stream.write("".join(chunk))
			chunk.clear()
	if chunk:
		stream.write("".join(chunk))


class ObjMeshAccumulator:
	__slots__ = ("vertex_registry", "uv_registry", "normal_registry", "vertices", "uvs", "normals", "faces")

	def __init__(self) -> None:
		self.vertex_registry: dict[ObjVectorKey, int] = {}
		self.uv_registry: dict[ObjUVKey, int] = {}
		self.normal_registry: dict[ObjVectorKey, int] = {}
		self.vertices: list[ObjVectorKey] = []
		self.uvs: list[ObjUVKey] = []
		self.normals: list[ObjVectorKey] = []
		self.faces: list[ObjFace] = []

	@property
	def triangle_count(self) -> int:
		return len(self.faces)

	def add_triangle(
		self,
		vertex_keys: tuple[ObjVectorKey, ObjVectorKey, ObjVectorKey],
		normal_keys: tuple[ObjVectorKey, ObjVectorKey, ObjVectorKey],
		uv_keys: tuple[ObjUVKey, ObjUVKey, ObjUVKey],
	) -> None:
		face: list[ObjFaceCorner] = []
		for index in range(3):
			vertex_index = obj_index_for_key(vertex_keys[index], self.vertex_registry, self.vertices)
			uv_index = obj_uv_index_for_key(uv_keys[index], self.uv_registry, self.uvs)
			normal_index = obj_index_for_key(normal_keys[index], self.normal_registry, self.normals)
			face.append((vertex_index, uv_index, normal_index))
		self.faces.append((face[0], face[1], face[2]))

	def write(self, mesh_path: Path) -> None:
		mesh_path.parent.mkdir(parents=True, exist_ok=True)
		with mesh_path.open("w", encoding="utf-8") as stream:
			stream.write("# Generated by tools/blender_export_luz.py\n")
			write_text_line_chunks(stream, (f"v {x} {y} {z}\n" for x, y, z in self.vertices))
			write_text_line_chunks(stream, (f"vt {u} {v}\n" for u, v in self.uvs))
			write_text_line_chunks(stream, (f"vn {x} {y} {z}\n" for x, y, z in self.normals))
			write_text_line_chunks(
				stream,
				(
					f"f {face[0][0]}/{face[0][1]}/{face[0][2]} "
					f"{face[1][0]}/{face[1][1]}/{face[1][2]} "
					f"{face[2][0]}/{face[2][1]}/{face[2][2]}\n"
					for face in self.faces
				),
			)


def export_meshes(
	args: argparse.Namespace,
	output_path: Path,
	mesh_dir: Path,
	texture_dir: Path,
	scene_texture_dir: Path,
	materials: dict[str, LuzMaterial],
	used_material_names: set[str],
	used_texture_names: set[str],
	exported_textures: dict[str, str],
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
			object_meshes_by_material: dict[int, ObjMeshAccumulator] = {}
			luz_vertex_keys: dict[int, ObjVectorKey] = {}
			luz_normal_keys: dict[int, ObjVectorKey] = {}
			luz_uv_keys: dict[int, ObjUVKey] = {}
			mesh_vertices = mesh.vertices
			mesh_loops = mesh.loops
			mesh_loop_triangles = mesh.loop_triangles
			mesh_corner_normals = getattr(mesh, "corner_normals", None)
			mesh_uv_layer_data = active_uv_layer_data(mesh)
			log_profile(
				args,
				f"[{object_index}/{len(mesh_objects)}] '{object_.name}' has "
				f"{len(mesh_vertices)} vertex/vertices and {len(mesh_loop_triangles)} triangle(s)",
			)

			def luz_vertex_key_for(vertex_index: int) -> ObjVectorKey:
				luz_vertex_key = luz_vertex_keys.get(vertex_index)
				if luz_vertex_key is None:
					blender_world = world_matrix @ mesh_vertices[vertex_index].co
					luz_vertex = blender_point_to_luz(blender_world, args.global_scale)
					bounds.include(luz_vertex)
					luz_vertex_key = obj_vector_key(luz_vertex)
					luz_vertex_keys[vertex_index] = luz_vertex_key
				return luz_vertex_key

			def luz_normal_key_for(loop_index: int) -> ObjVectorKey:
				luz_normal_key = luz_normal_keys.get(loop_index)
				if luz_normal_key is None:
					luz_normal = loop_normal_from_data(mesh_corner_normals, mesh_loops, loop_index, normal_matrix)
					luz_normal_key = obj_vector_key(luz_normal)
					luz_normal_keys[loop_index] = luz_normal_key
				return luz_normal_key

			def luz_uv_key_for(loop_index: int) -> ObjUVKey:
				luz_uv_key = luz_uv_keys.get(loop_index)
				if luz_uv_key is None:
					luz_uv = loop_uv_from_data(mesh_uv_layer_data, loop_index)
					luz_uv_key = obj_uv_key(luz_uv)
					luz_uv_keys[loop_index] = luz_uv_key
				return luz_uv_key

			for loop_triangle in mesh_loop_triangles:
				material_index = loop_triangle.material_index
				obj_mesh = object_meshes_by_material.get(material_index)
				if obj_mesh is None:
					obj_mesh = ObjMeshAccumulator()
					object_meshes_by_material[material_index] = obj_mesh
				loop_indices = loop_triangle.loops
				vertex_indices = loop_triangle.vertices
				obj_mesh.add_triangle(
					(
						luz_vertex_key_for(vertex_indices[0]),
						luz_vertex_key_for(vertex_indices[1]),
						luz_vertex_key_for(vertex_indices[2]),
					),
					(
						luz_normal_key_for(loop_indices[0]),
						luz_normal_key_for(loop_indices[1]),
						luz_normal_key_for(loop_indices[2]),
					),
					(
						luz_uv_key_for(loop_indices[0]),
						luz_uv_key_for(loop_indices[1]),
						luz_uv_key_for(loop_indices[2]),
					),
				)

			for material_index, obj_mesh in object_meshes_by_material.items():
				if obj_mesh.triangle_count == 0:
					continue

				material = material_for_slot(object_, material_index)
				material_label = getattr(material, "name_full", "default material") if material else "default material"
				material_start = time.perf_counter()
				log_profile(
					args,
					f"[{object_index}/{len(mesh_objects)}] Resolving material '{material_label}' "
					f"for {obj_mesh.triangle_count} triangle(s)",
				)
				material_name = export_material(
					material,
					used_material_names,
					materials,
					texture_dir,
					scene_texture_dir,
					used_texture_names,
					exported_textures,
					args,
				)
				mesh_base = slugify(f"{object_.name}_{material_name}", "mesh")
				mesh_name = unique_name(mesh_base, used_mesh_names)
				object_name = unique_name(slugify(f"{object_.name}_{material_name}", "object"), used_object_names)
				mesh_path = mesh_dir / f"{mesh_name}.obj"
				scene_file_path = Path(os.path.relpath(mesh_path, output_path.parent)).as_posix()

				log_profile(args, f"[{object_index}/{len(mesh_objects)}] Writing {mesh_path.name}")
				obj_mesh.write(mesh_path)
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
						triangle_count=obj_mesh.triangle_count,
					)
				)
		finally:
			release_evaluated_mesh(evaluated_object)
		log_profile(args, f"[{object_index}/{len(mesh_objects)}] Finished '{object_.name}' in {time.perf_counter() - object_start:.2f}s")

	return meshes


def export_camera(args: argparse.Namespace, bounds: Bounds) -> LuzCamera:
	scene = bpy.context.scene
	camera_object = scene.camera
	unit_scale = blender_meters_per_unit()

	if camera_object and camera_object.type == "CAMERA":
		camera_data = camera_object.data
		position = blender_point_to_luz(camera_object.matrix_world.translation, args.global_scale)
		direction = blender_vector_to_luz(camera_object.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0)))
		up = blender_vector_to_luz(camera_object.matrix_world.to_quaternion() @ Vector((0.0, 1.0, 0.0)))
		focal_length_mm = max(float(getattr(camera_data, "lens", 50.0)), 1e-6)
		sensor_width_mm = max(float(getattr(camera_data, "sensor_width", 36.0)), 1e-6)
		sensor_height_mm = max(float(getattr(camera_data, "sensor_height", 20.25)), 1e-6)
		sensor_width_mm, sensor_height_mm = effective_camera_sensor_mm(
			camera_data,
			scene,
			focal_length_mm,
			sensor_width_mm,
			sensor_height_mm,
		)

		use_dof = getattr(camera_data.dof, "use_dof", False)
		focus_distance_meters = float(args.default_focus_distance) * unit_scale
		has_valid_blender_focus = False
		if use_dof:
			focus_object = getattr(camera_data.dof, "focus_object", None)
			if focus_object:
				focus_distance_meters = (focus_object.matrix_world.translation - camera_object.matrix_world.translation).length * unit_scale
				has_valid_blender_focus = focus_distance_meters > 1e-6
			else:
				focus_distance_meters = float(camera_data.dof.focus_distance) * unit_scale
				has_valid_blender_focus = focus_distance_meters > 1e-6

		if focus_distance_meters <= 1e-6:
			center_distance = (bounds.center() - position).dot(direction)
			if center_distance > 1e-6:
				focus_distance_meters = (center_distance / args.global_scale) * unit_scale
			else:
				focus_distance_meters = max(float(args.default_focus_distance) * unit_scale, unit_scale)

		if args.camera_f_stop is not None:
			f_stop = args.camera_f_stop
			pinhole = False
		elif use_dof and has_valid_blender_focus and getattr(camera_data.dof, "aperture_fstop", 0.0) > 0.0:
			f_stop = float(camera_data.dof.aperture_fstop)
			pinhole = False
		else:
			f_stop = 8.0
			pinhole = True

		return LuzCamera("main", position, direction, up, focal_length_mm, sensor_width_mm, sensor_height_mm, f_stop, pinhole, focus_distance_meters)

	center = bounds.center()
	radius = bounds.radius()
	position = Vector((center.x, center.y + radius * 0.25, center.z + radius * 3.0))
	direction = center - position
	focus_distance_meters = max((direction.length / args.global_scale) * unit_scale, unit_scale)
	return LuzCamera("main", position, direction.normalized(), Vector((0.0, 1.0, 0.0)), 50.0, 36.0, 20.25, 8.0, True, focus_distance_meters)


def light_color(light_data: object) -> tuple[float, float, float]:
	return color_from_value(getattr(light_data, "color", (1.0, 1.0, 1.0)), (1.0, 1.0, 1.0))


def blender_light_power(energy: float, scale: float) -> float:
	return max(energy * scale, 0.0)


def blender_sun_irradiance(energy: float, scale: float) -> float:
	return max(energy * scale, 0.0)


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

		if data.type == "AREA":
			blender_width = float(getattr(data, "size", 1.0))
			blender_height = blender_width
			if getattr(data, "shape", "") in {"RECTANGLE", "ELLIPSE"}:
				blender_height = float(getattr(data, "size_y", blender_width))
			width = blender_width * args.global_scale
			height = blender_height * args.global_scale
			quantity = blender_light_power(energy, args.light_power_scale)
			log_profile(
				args,
				f"Light '{object_.name}' type={data.type} energy={fmt_float(energy)} "
				f"power={fmt_float(quantity)} direction={fmt_vector(direction)}",
			)
			lights.append(
				LuzLight(
					kind="area_light",
					name=name,
					position=position,
					normal=direction,
					width=max(width, 1e-6),
					height=max(height, 1e-6),
					color=color,
					quantity=quantity,
					unit_key="power",
				)
			)
		elif data.type == "SUN":
			quantity = blender_sun_irradiance(energy, args.sun_power_scale)
			log_profile(
				args,
				f"Light '{object_.name}' type={data.type} energy={fmt_float(energy)} "
				f"irradiance={fmt_float(quantity)} direction={fmt_vector(direction)}",
			)
			lights.append(
				LuzLight(
					kind="directional_light",
					name=name,
					normal=direction,
					color=color,
					quantity=quantity,
					unit_key="irradiance",
					atmosphere_angle=atmosphere_angle_from_sun_direction(direction),
				)
			)
		else:
			blender_radius = max(
				float(getattr(data, "shadow_soft_size", args.min_point_light_radius)),
				args.min_point_light_radius,
				1e-6,
			)
			radius = max(blender_radius * args.global_scale, 1e-6)
			quantity = blender_light_power(energy, args.light_power_scale)
			log_profile(
				args,
				f"Light '{object_.name}' type={data.type} energy={fmt_float(energy)} "
				f"power={fmt_float(quantity)} direction={fmt_vector(direction)}",
			)
			lights.append(
				LuzLight(
					kind="point_light",
					name=name,
					position=position,
					radius=radius,
					color=color,
					quantity=quantity,
					unit_key="power",
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


def world_environment_from_shader_node(
	node: object | None,
	camera_ray: bool,
	visited: set[object] | None = None,
) -> tuple[object, float] | None:
	if node is None:
		return None
	if visited is None:
		visited = set()
	if node in visited:
		return None
	visited.add(node)

	node_type = getattr(node, "type", "")
	if node_type in {"BACKGROUND", "EMISSION"}:
		image = texture_image_from_socket(node_socket(node, "Color"))
		if image is None:
			return None
		strength = scalar_from_value(socket_default_value(node, ("Strength",), 1.0), 1.0)
		return (image, max(strength, 0.0))

	if node_type == "MIX_SHADER":
		nodes = linked_shader_nodes(node)
		if len(nodes) >= 2:
			if mix_shader_uses_camera_ray_factor(node):
				branch = nodes[1] if camera_ray else nodes[0]
				return world_environment_from_shader_node(branch, camera_ray, visited.copy())
			first = world_environment_from_shader_node(nodes[0], camera_ray, visited.copy())
			if first is not None:
				return first
			return world_environment_from_shader_node(nodes[1], camera_ray, visited.copy())

	for child in linked_shader_nodes(node):
		environment = world_environment_from_shader_node(child, camera_ray, visited.copy())
		if environment is not None:
			return environment
	return None


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


def export_world_environment(
	args: argparse.Namespace,
	texture_dir: Path,
	scene_texture_dir: Path,
	used_texture_names: set[str],
	exported_textures: dict[str, str],
) -> LuzEnvironment | None:
	world = bpy.context.scene.world
	background_node = world_surface_node(world)
	environment = world_environment_from_shader_node(background_node, True)
	if environment is None:
		return None

	image, strength = environment
	path = export_environment_image(
		image,
		texture_dir,
		scene_texture_dir,
		used_texture_names,
		exported_textures,
		args,
	)
	if path is None:
		return None
	return LuzEnvironment(path=path, strength=strength)


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


def bool_from_mode(mode: str, auto_value: bool) -> bool:
	if mode in {"on", "true", "1"}:
		return True
	if mode in {"off", "false", "0"}:
		return False
	return auto_value


def cycles_bool(name: str, fallback: bool) -> bool:
	cycles = getattr(bpy.context.scene, "cycles", None)
	if cycles is None or not hasattr(cycles, name):
		return fallback
	try:
		return bool(getattr(cycles, name))
	except Exception:
		return fallback


def cycles_positive_int(name: str) -> int | None:
	cycles = getattr(bpy.context.scene, "cycles", None)
	if cycles is None or not hasattr(cycles, name):
		return None
	try:
		value = int(getattr(cycles, name))
	except Exception:
		return None
	if value <= 0:
		return None
	return value


def cycles_positive_float(name: str) -> float | None:
	cycles = getattr(bpy.context.scene, "cycles", None)
	if cycles is None or not hasattr(cycles, name):
		return None
	try:
		value = float(getattr(cycles, name))
	except Exception:
		return None
	if not math.isfinite(value) or value <= 0.0:
		return None
	return value


def scene_adaptive(args: argparse.Namespace) -> bool:
	return bool_from_mode(args.adaptive, cycles_bool("use_adaptive_sampling", True))


def scene_adaptive_min_samples(args: argparse.Namespace, samples: int) -> int:
	if args.adaptive_min_samples is not None:
		return args.adaptive_min_samples
	cycles_min = cycles_positive_int("adaptive_min_samples")
	if cycles_min is not None:
		return cycles_min
	return min(samples, max(16, samples // 4))


def scene_adaptive_threshold(args: argparse.Namespace) -> float:
	if args.adaptive_threshold is not None:
		return args.adaptive_threshold
	cycles_threshold = cycles_positive_float("adaptive_threshold")
	if cycles_threshold is not None:
		return cycles_threshold
	return 0.02


def scene_adaptive_check_interval(args: argparse.Namespace, adaptive_min_samples: int) -> int:
	if args.adaptive_check_interval is not None:
		return args.adaptive_check_interval
	return min(64, max(8, adaptive_min_samples // 4))


def scene_denoise(args: argparse.Namespace) -> bool:
	return bool_from_mode(args.denoise, cycles_bool("use_denoising", True))


def scene_view_settings() -> object | None:
	return getattr(bpy.context.scene, "view_settings", None)


def scene_exposure(args: argparse.Namespace) -> float:
	if args.exposure is not None:
		return args.exposure
	view_settings = scene_view_settings()
	if view_settings is None:
		return 0.0
	try:
		exposure = float(getattr(view_settings, "exposure", 0.0))
	except Exception:
		return 0.0
	return exposure if math.isfinite(exposure) else 0.0


def scene_view_transform(args: argparse.Namespace) -> str:
	if args.view_transform != "auto":
		return args.view_transform
	view_settings = scene_view_settings()
	view_transform = ""
	if view_settings is not None:
		try:
			view_transform = str(getattr(view_settings, "view_transform", ""))
		except Exception:
			view_transform = ""
	view_transform = view_transform.strip().lower()
	if view_transform == "agx":
		return "agx"
	if view_transform == "aces":
		return "aces"
	if view_transform in {"standard", "raw"}:
		return "standard"
	return "agx"


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
	environment: LuzEnvironment | None,
) -> None:
	width, height = scene_resolution(args)
	samples = scene_samples(args)
	adaptive = scene_adaptive(args)
	adaptive_min_samples = scene_adaptive_min_samples(args, samples)
	render_output = args.render_output
	if not render_output:
		render_output = str(output_path.with_suffix("")) + "_render"
	sky = args.sky or ("environment" if environment is not None else "none")

	lines: list[str] = []
	lines.extend(
		[
			"[settings]",
			f"resolution={width},{height}",
			f"samples={samples}",
			f"adaptive={1 if adaptive else 0}",
			f"maxlightbounces={scene_bounces(args)}",
			f"meters_per_unit={fmt_float(luz_meters_per_unit(args))}",
			f"view_transform={scene_view_transform(args)}",
			"bloom=1",
			f"exposure={fmt_float(scene_exposure(args))}",
			f"denoise={1 if scene_denoise(args) else 0}",
			f"sky={sky}",
		]
	)
	if adaptive:
		lines.extend(
			[
				f"adaptiveminsamples={adaptive_min_samples}",
				f"adaptivethreshold={fmt_float(scene_adaptive_threshold(args))}",
				f"adaptivecheckinterval={scene_adaptive_check_interval(args, adaptive_min_samples)}",
			]
		)
	if sky == "atmosphere":
		lines.append(
			f"atmosphere={fmt_float(exported_atmosphere_angle(lights))},"
			"6360000,6420000,7994,1200,16,8,0.5"
		)
	if environment is not None:
		lines.append(
			f"environment={environment.path},"
			f"{fmt_float(environment.rotation)}"
		)
		lines.append(f"environment_scale={fmt_float(environment.strength)}")
	lines.extend(
		[
			f"background={fmt_blender_color(background_color)}",
			f"outputfilename={render_output}",
			"",
		]
	)

	if materials:
		lines.append("[materials]")
		for material in materials.values():
			lines.extend([f"material {material.name} {{", f"type={material.material_type}"])
			if material.material_type == "emissive":
				lines.extend(
					[
						f"color={fmt_blender_color(material.emission_color)}",
						f"radiance={fmt_float(material.emission_strength)}",
					]
				)
			else:
				lines.extend(
					[
						f"base_color={fmt_blender_color(material.base_color)}",
						f"roughness={fmt_float(material.roughness)}",
					]
				)
				if material.material_type == "diffuse_glossy":
					lines.extend(
						[
							f"glossy_color={fmt_blender_color(material.glossy_color)}",
							f"glossy_weight={fmt_float(material.glossy_weight)}",
						]
					)
				if material.material_type in {"principled", "metal"}:
					lines.append(f"metallic={fmt_float(material.metallic)}")
				if material.material_type in {"principled", "dielectric"}:
					lines.extend(
						[
							f"alpha={fmt_float(material.alpha)}",
							f"transmission={fmt_float(material.transmission)}",
						]
					)
					lines.append(f"ior={fmt_float(material.refractive_index)}")
				if material.material_type == "principled":
					lines.extend(
						[
							f"clearcoat={fmt_float(material.clearcoat)}",
							f"clearcoat_roughness={fmt_float(material.clearcoat_roughness)}",
							f"sheen={fmt_float(material.sheen)}",
						]
					)
					if material.subsurface > 0.0:
						lines.extend(
							[
								f"subsurface={fmt_float(material.subsurface)}",
								f"subsurface_method={material.subsurface_method}",
								f"subsurface_radius={fmt_color(material.subsurface_radius)}",
								f"subsurface_scale={fmt_float(material.subsurface_scale)}",
								f"subsurface_color={fmt_blender_color(material.subsurface_color)}",
							]
						)
			if material.texture_path:
				lines.append(f"texture={material.texture_path}")
			lines.append("}")
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
			f"focal_length_mm={fmt_float(camera.focal_length_mm)}",
			f"sensor_width_mm={fmt_float(camera.sensor_width_mm)}",
			f"sensor_height_mm={fmt_float(camera.sensor_height_mm)}",
			f"f_stop={fmt_float(camera.f_stop)}",
			f"pinhole={1 if camera.pinhole else 0}",
			f"focus_distance={fmt_float(camera.focus_distance_meters)}",
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
					f"color={fmt_blender_color(light.color)}",
					f"{light.unit_key}={fmt_float(light.quantity)}",
					"}",
				]
			)
		elif light.kind == "directional_light":
			lines.extend(
				[
					f"directional_light {light.name} {{",
					f"direction={fmt_vector(light.normal or Vector((0.0, -1.0, 0.0)))}",
					f"color={fmt_blender_color(light.color)}",
					f"{light.unit_key}={fmt_float(light.quantity)}",
					"}",
				]
			)
		else:
			lines.extend(
					[
						f"point_light {light.name} {{",
						f"position={fmt_vector(light.position or Vector((0.0, 0.0, 0.0)))}",
					f"radius={fmt_float(light.radius)}",
					f"color={fmt_blender_color(light.color)}",
					f"{light.unit_key}={fmt_float(light.quantity)}",
					"visible=0",
					"}",
				]
			)

	output_path.parent.mkdir(parents=True, exist_ok=True)
	output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
	global SAMPLE_TEXTURE_COLORS, TEXTURE_SAMPLE_SIZE
	if bpy is None:
		raise SystemExit("This exporter must be run inside Blender with --python.") from _BLENDER_IMPORT_ERROR
	start_time = time.perf_counter()
	args = parse_args(sys.argv)
	SAMPLE_TEXTURE_COLORS = args.sample_texture_colors
	TEXTURE_SAMPLE_SIZE = max(int(args.texture_sample_size), 1)
	IMAGE_AVERAGE_CACHE.clear()
	if args.blend:
		log_profile(args, f"Opening blend file {args.blend}")
		bpy.ops.wm.open_mainfile(filepath=args.blend)

	output_path = Path(args.output).expanduser().resolve()
	mesh_dir = Path(args.mesh_dir).expanduser()
	if not mesh_dir.is_absolute():
		mesh_dir = output_path.parent / mesh_dir
	texture_dir_arg = Path(args.texture_dir).expanduser()
	texture_dir = texture_dir_arg if texture_dir_arg.is_absolute() else output_path.parent / texture_dir_arg
	scene_texture_dir = texture_dir if texture_dir_arg.is_absolute() else texture_dir_arg

	materials: dict[str, LuzMaterial] = {}
	used_material_names: set[str] = set()
	used_texture_names: set[str] = set()
	exported_textures: dict[str, str] = {}
	bounds = Bounds()
	log_profile(args, "Starting mesh export")
	meshes = export_meshes(
		args,
		output_path,
		mesh_dir,
		texture_dir,
		scene_texture_dir,
		materials,
		used_material_names,
		used_texture_names,
		exported_textures,
		bounds,
	)
	log_profile(args, "Exporting camera")
	camera = export_camera(args, bounds)
	lights = export_lights(args, bounds)
	background_color = export_world_background()
	log_profile(args, f"World camera background={fmt_color(background_color)}")
	environment = export_world_environment(
		args,
		texture_dir,
		scene_texture_dir,
		used_texture_names,
		exported_textures,
	)
	if environment is not None:
		log_profile(args, f"World environment={environment.path}")
	log_profile(args, f"Writing Luz scene {output_path}")
	write_luz_scene(args, output_path, materials, meshes, camera, lights, background_color, environment)

	triangle_count = sum(mesh.triangle_count for mesh in meshes)
	print(
		f"Exported {len(meshes)} mesh group(s), {triangle_count} triangle(s), "
		f"{len(materials)} material(s), and {len(lights)} light(s) to {output_path} "
		f"in {time.perf_counter() - start_time:.2f}s"
	)


if __name__ == "__main__":
	main()
