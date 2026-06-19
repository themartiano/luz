#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path


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


def main() -> None:
	test_environment_texture_survives_color_ramp_and_mix_rgb()
	test_environment_texture_survives_modern_mix_node_inputs()


if __name__ == "__main__":
	main()
