#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("blender_export_luz", ROOT / "tools" / "blender_export_luz.py")
assert SPEC is not None and SPEC.loader is not None
exporter = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = exporter
SPEC.loader.exec_module(exporter)


class SocketCollection(dict[str, "FakeSocket"]):
	def __iter__(self):
		return iter(self.values())


class FakeLink:
	def __init__(self, from_socket: "FakeSocket") -> None:
		self.from_socket = from_socket
		self.from_node = from_socket.node


class FakeSocket:
	def __init__(self, node: "FakeNode", name: str) -> None:
		self.node = node
		self.name = name
		self.links: list[FakeLink] = []


class FakeNode:
	def __init__(self, node_type: str, image: object | None = None) -> None:
		self.type = node_type
		self.image = image
		self.inputs = SocketCollection()
		self.outputs = SocketCollection()

	def input(self, name: str) -> FakeSocket:
		socket = FakeSocket(self, name)
		self.inputs[name] = socket
		return socket

	def output(self, name: str) -> FakeSocket:
		socket = FakeSocket(self, name)
		self.outputs[name] = socket
		return socket


def link(from_socket: FakeSocket, to_socket: FakeSocket) -> None:
	to_socket.links.append(FakeLink(from_socket))


class FakeImage:
	def __init__(self, pixels: object, size: tuple[int, int]) -> None:
		self.pixels = pixels
		self.size = size


class CountingPixels:
	def __init__(self, values: list[float]) -> None:
		self.values = values
		self.read_count = 0

	def foreach_get(self, target: object) -> None:
		self.read_count += 1
		for index, value in enumerate(self.values):
			target[index] = value  # type: ignore[index]


class FakeFramePoint:
	def __init__(self, x: float, y: float, z: float) -> None:
		self.x = x
		self.y = y
		self.z = z


class FakeCameraData:
	type = "PERSP"

	def __init__(self, frame: list[FakeFramePoint]) -> None:
		self.frame = frame

	def view_frame(self, scene: object) -> list[FakeFramePoint]:
		return self.frame


def test_parse_args_accepts_subsurface_scale_multiplier() -> None:
	args = exporter.parse_args(["--", "--output", "scene.luz", "--subsurface-scale-multiplier", "2.5"])

	assert args.subsurface_scale_multiplier == 2.5


def test_luz_meters_per_unit_respects_blender_unit_scale() -> None:
	old_bpy = exporter.bpy
	try:
		exporter.bpy = SimpleNamespace(
			context=SimpleNamespace(
				scene=SimpleNamespace(
					unit_settings=SimpleNamespace(scale_length=0.1),
				),
			),
		)
		assert abs(exporter.luz_meters_per_unit(SimpleNamespace(global_scale=2.0)) - 0.05) < 1e-12
	finally:
		exporter.bpy = old_bpy


def test_environment_texture_survives_color_ramp_and_mix_rgb() -> None:
	image = object()
	environment = FakeNode("TEX_ENVIRONMENT", image)
	environment_color = environment.output("Color")
	color_ramp = FakeNode("VALTORGB")
	color_ramp_fac = color_ramp.input("Fac")
	color_ramp_color = color_ramp.output("Color")
	mix = FakeNode("MIX_RGB")
	mix_color = mix.input("Color1")
	mix_output = mix.output("Color")

	link(environment_color, color_ramp_fac)
	link(color_ramp_color, mix_color)

	assert exporter.texture_image_from_output(mix_output, set()) is image


def test_environment_texture_survives_modern_mix_node_inputs() -> None:
	image = object()
	environment = FakeNode("TEX_ENVIRONMENT", image)
	environment_color = environment.output("Color")
	mix = FakeNode("MIX")
	mix.input("A")
	mix_b = mix.input("B")
	mix_output = mix.output("Result")

	link(environment_color, mix_b)

	assert exporter.texture_image_from_output(mix_output, set()) is image


def test_separate_color_channel_resolves_image_average_for_scalar_inputs() -> None:
	exporter.IMAGE_AVERAGE_CACHE.clear()
	image = FakeImage(
		[
			0.1,
			0.6,
			0.2,
			1.0,
			0.3,
			0.8,
			0.4,
			1.0,
		],
		(2, 1),
	)
	texture = FakeNode("TEX_IMAGE", image)
	texture_color = texture.output("Color")
	separate = FakeNode("SEPARATE_COLOR")
	separate_color = separate.input("Color")
	separate_green = separate.output("Green")
	principled = FakeNode("BSDF_PRINCIPLED")
	roughness = principled.input("Roughness")

	link(texture_color, separate_color)
	link(separate_green, roughness)

	value = exporter.scalar_from_value(exporter.resolve_socket_value(roughness, 0.5), 0.5)
	assert abs(value - 0.7) < 1e-12


def test_average_image_color_caches_pixel_reads() -> None:
	exporter.IMAGE_AVERAGE_CACHE.clear()
	pixels = CountingPixels(
		[
			0.25,
			0.5,
			0.75,
			1.0,
			0.75,
			0.5,
			0.25,
			1.0,
		]
	)
	image = FakeImage(pixels, (2, 1))

	assert exporter.average_image_color(image) == (0.5, 0.5, 0.5)
	assert exporter.average_image_color(image) == (0.5, 0.5, 0.5)
	assert pixels.read_count == 1
	exporter.IMAGE_AVERAGE_CACHE.clear()


def test_write_image_as_ppm_writes_buffered_bottom_up_rgb() -> None:
	image = FakeImage(
		[
			1.0,
			0.0,
			0.0,
			1.0,
			0.0,
			1.0,
			0.0,
			1.0,
			0.0,
			0.0,
			1.0,
			1.0,
			1.0,
			1.0,
			1.0,
			1.0,
		],
		(2, 2),
	)
	with TemporaryDirectory() as directory:
		path = Path(directory) / "texture.ppm"
		assert exporter.write_image_as_ppm(image, path, 1024)
		assert path.read_bytes() == (
			b"P6\n2 2\n255\n"
			+ bytes(
				[
					0,
					0,
					255,
					255,
					255,
					255,
					255,
					0,
					0,
					0,
					255,
					0,
				]
			)
		)


def test_obj_mesh_accumulator_deduplicates_indices() -> None:
	obj_mesh = exporter.ObjMeshAccumulator()
	vertices = (("0", "0", "0"), ("1", "0", "0"), ("0", "1", "0"))
	normals = (("0", "0", "1"), ("0", "0", "1"), ("0", "0", "1"))
	uvs = (("0", "0"), ("1", "0"), ("0", "1"))

	obj_mesh.add_triangle(vertices, normals, uvs)
	obj_mesh.add_triangle(vertices, normals, uvs)

	assert obj_mesh.triangle_count == 2
	assert obj_mesh.vertices == list(vertices)
	assert obj_mesh.normals == [("0", "0", "1")]
	assert obj_mesh.uvs == list(uvs)
	assert obj_mesh.faces == [
		((1, 1, 1), (2, 2, 1), (3, 3, 1)),
		((1, 1, 1), (2, 2, 1), (3, 3, 1)),
	]
	with TemporaryDirectory() as directory:
		path = Path(directory) / "mesh.obj"
		obj_mesh.write(path)
		assert path.read_text(encoding="utf-8") == (
			"# Generated by tools/blender_export_luz.py\n"
			"v 0 0 0\n"
			"v 1 0 0\n"
			"v 0 1 0\n"
			"vt 0 0\n"
			"vt 1 0\n"
			"vt 0 1\n"
			"vn 0 0 1\n"
			"f 1/1/1 2/2/1 3/3/1\n"
			"f 1/1/1 2/2/1 3/3/1\n"
		)


def test_effective_camera_sensor_uses_blender_view_frame() -> None:
	camera = FakeCameraData(
		[
			FakeFramePoint(0.5, 0.5, -2.7777777778),
			FakeFramePoint(0.5, -0.5, -2.7777777778),
			FakeFramePoint(-0.5, -0.5, -2.7777777778),
			FakeFramePoint(-0.5, 0.5, -2.7777777778),
		]
	)

	width, height = exporter.effective_camera_sensor_mm(camera, object(), 100.0, 36.0, 24.0)

	assert abs(width - 36.0) < 1e-9
	assert abs(height - 36.0) < 1e-9


def main() -> None:
	test_parse_args_accepts_subsurface_scale_multiplier()
	test_luz_meters_per_unit_respects_blender_unit_scale()
	test_environment_texture_survives_color_ramp_and_mix_rgb()
	test_environment_texture_survives_modern_mix_node_inputs()
	test_separate_color_channel_resolves_image_average_for_scalar_inputs()
	test_average_image_color_caches_pixel_reads()
	test_write_image_as_ppm_writes_buffered_bottom_up_rgb()
	test_obj_mesh_accumulator_deduplicates_indices()
	test_effective_camera_sensor_uses_blender_view_frame()


if __name__ == "__main__":
	main()
