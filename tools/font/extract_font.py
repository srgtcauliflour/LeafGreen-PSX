#!/usr/bin/env python3
"""Extract a byte-verified normal Latin atlas from the exact local Rev 1 ROM."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'romverify'))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'gfxconv'))
from verify_leafgreen import EXPECTED_SHA1, EXPECTED_SIZE
from tim import tim4

FONT_OFFSET = 0x1F314C
WIDTH_OFFSET = 0x1FB14C
FONT_SHA256 = '1575dd18118133025bc2a70cdb7bdd15d65932e3e82fe49a03178290d68ec25f'
WIDTH_SHA256 = '0bb98329c095a8cac9224155634263e9a58e31269d7f9040e5b5d590ffd78238'


def decode_glyph(data):
    if len(data) != 64:
        raise ValueError('A full-width glyph must contain 64 bytes')
    pixels = [0] * 256
    for tile in range(4):
        for row in range(8):
            bits = int.from_bytes(data[tile * 16 + row * 2:tile * 16 + row * 2 + 2], 'little')
            for column in range(8):
                x = (tile % 2) * 8 + column
                y = (tile // 2) * 8 + row
                pixels[y * 16 + x] = (bits >> (14 - column * 2)) & 3
    return pixels


def build_atlas(font):
    if len(font) != 256 * 64:
        raise ValueError('Atlas requires exactly 256 glyphs')
    pixels = bytearray(256 * 256)
    # Reference glyph zero is a blank space, regardless of stored graphic.
    for glyph in range(1, 256):
        decoded = decode_glyph(font[glyph * 64:(glyph + 1) * 64])
        for y in range(16):
            start = ((glyph // 16) * 16 + y) * 256 + (glyph % 16) * 16
            pixels[start:start + 16] = bytes(decoded[y * 16:y * 16 + 16])
    return bytes(pixels[i] | pixels[i + 1] << 4 for i in range(0, len(pixels), 2))


def extract(rom, output):
    if len(rom) != EXPECTED_SIZE or hashlib.sha1(rom).hexdigest() != EXPECTED_SHA1:
        raise ValueError('ROM does not match LeafGreen USA Rev 1')
    font = rom[FONT_OFFSET:FONT_OFFSET + 32768]
    widths = rom[WIDTH_OFFSET:WIDTH_OFFSET + 512]
    for data, expected in ((font, FONT_SHA256), (widths, WIDTH_SHA256)):
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError('Verified font descriptor does not match ROM bytes')
    atlas = build_atlas(font[:16384])
    # Original diagnostic style: transparent background, white ink, dark shadow.
    palette = [0, 0x7FFF, 0x8421, 0] + [0] * 12
    output.mkdir(parents=True, exist_ok=True)
    (output / 'normal.tim').write_bytes(tim4(atlas, palette, 256, 256))
    arrays = [('uint32_t', 'lg_font_pixels', struct.unpack('<8192I', atlas)),
              ('uint16_t', 'lg_font_palette', palette),
              ('uint8_t', 'lg_font_widths', widths[:256])]
    header = '#pragma once\n#include <stdint.h>\n/* Generated from the local ROM. Never commit. */\n'
    for kind, name, values in arrays:
        header += f'_Alignas(4) static const {kind} {name}[{len(values)}] = {{\n'
        for start in range(0, len(values), 16):
            header += ','.join(str(v) for v in values[start:start + 16]) + ',\n'
        header += '};\n'
    (output / 'font_data.h').write_text(header)
    (output / 'font_manifest.json').write_text(json.dumps({
        'target_sha1': EXPECTED_SHA1, 'font_offset': FONT_OFFSET,
        'width_offset': WIDTH_OFFSET, 'font_sha256': FONT_SHA256,
        'width_sha256': WIDTH_SHA256, 'atlas_glyphs': 256,
        'cell': [16, 16], 'text_height': 14,
        'atlas_sha256': hashlib.sha256(atlas).hexdigest()}, indent=2) + '\n')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--output', type=Path, default=Path('generated/font'))
    args = parser.parse_args()
    try:
        extract(args.rom.read_bytes(), args.output)
    except (OSError, ValueError) as exc:
        parser.exit(1, f'FAIL: {exc}\n')
    print('PASS: verified font atlas generated locally')
