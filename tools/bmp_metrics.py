#!/usr/bin/env python3
import math
import struct
import sys


def read_bmp(path):
    with open(path, "rb") as handle:
        data = handle.read()

    if len(data) < 54 or data[0:2] != b"BM":
        raise ValueError(f"{path}: not a BMP file")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError(f"{path}: unsupported BMP DIB header")

    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    planes = struct.unpack_from("<H", data, 26)[0]
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]

    if planes != 1 or compression != 0 or bits_per_pixel not in (24, 32):
        raise ValueError(f"{path}: only uncompressed 24/32-bit BMPs are supported")
    if width <= 0 or height == 0:
        raise ValueError(f"{path}: invalid BMP dimensions")

    abs_height = abs(height)
    bytes_per_pixel = bits_per_pixel // 8
    row_stride = ((width * bits_per_pixel + 31) // 32) * 4
    top_down = height < 0
    pixels = []

    for y in range(abs_height):
        source_y = y if top_down else abs_height - 1 - y
        row_start = pixel_offset + source_y * row_stride
        row = []
        for x in range(width):
            offset = row_start + x * bytes_per_pixel
            if offset + bytes_per_pixel > len(data):
                raise ValueError(f"{path}: truncated BMP pixel data")
            blue, green, red = data[offset], data[offset + 1], data[offset + 2]
            row.append((red, green, blue))
        pixels.append(row)

    return width, abs_height, pixels


def compare(reference_path, candidate_path):
    ref_width, ref_height, reference = read_bmp(reference_path)
    cand_width, cand_height, candidate = read_bmp(candidate_path)
    if (ref_width, ref_height) != (cand_width, cand_height):
        raise ValueError(
            f"image dimensions differ: {ref_width}x{ref_height} vs {cand_width}x{cand_height}"
        )

    count = ref_width * ref_height * 3
    sum_abs = 0
    sum_squared = 0
    max_abs = 0

    for y in range(ref_height):
        for x in range(ref_width):
            ref_pixel = reference[y][x]
            cand_pixel = candidate[y][x]
            for channel in range(3):
                diff = cand_pixel[channel] - ref_pixel[channel]
                abs_diff = abs(diff)
                sum_abs += abs_diff
                sum_squared += diff * diff
                max_abs = max(max_abs, abs_diff)

    mae = sum_abs / count
    mse = sum_squared / count
    rmse = math.sqrt(mse)
    psnr = float("inf") if mse == 0.0 else 20.0 * math.log10(255.0 / rmse)

    return {
        "width": ref_width,
        "height": ref_height,
        "mae": mae,
        "rmse": rmse,
        "psnr": psnr,
        "max_abs": max_abs,
    }


def main(argv):
    if len(argv) != 3:
        print(f"Usage: {argv[0]} REFERENCE.bmp CANDIDATE.bmp", file=sys.stderr)
        return 2

    try:
        metrics = compare(argv[1], argv[2])
    except ValueError as error:
        print(error, file=sys.stderr)
        return 1

    print(
        "width={width} height={height} mae={mae:.6f} rmse={rmse:.6f} "
        "psnr={psnr:.6f} max_abs={max_abs}".format(**metrics)
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
