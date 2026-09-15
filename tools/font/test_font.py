import tempfile
from pathlib import Path
import unittest
from extract_font import decode_glyph, build_atlas, extract

class FontTests(unittest.TestCase):
    def test_pixel_order_and_quadrants(self):
        data = bytearray(64)
        data[0:2] = (0x1b1b).to_bytes(2, 'little')
        data[16:18] = (0xffff).to_bytes(2, 'little')
        data[32:34] = (0x5555).to_bytes(2, 'little')
        data[48:50] = (0xaaaa).to_bytes(2, 'little')
        pixels = decode_glyph(data)
        self.assertEqual(pixels[:8], [0,1,2,3,0,1,2,3])
        self.assertEqual(pixels[8:16], [3]*8)
        self.assertEqual(pixels[128:144], [1]*8 + [2]*8)

    def test_atlas_and_blank_space(self):
        font = bytearray([255] * (256 * 64))
        atlas = build_atlas(font)
        self.assertEqual(len(atlas), 32768)
        self.assertEqual(atlas[:8], bytes(8))
        self.assertEqual(atlas[8:16], bytes([0x33]*8))
        self.assertEqual(atlas[-8:], bytes([0x33]*8))

    def test_bad_lengths_and_wrong_rom(self):
        with self.assertRaises(ValueError): decode_glyph(bytes(63))
        with self.assertRaises(ValueError): build_atlas(bytes(64))
        with tempfile.TemporaryDirectory() as temp:
            out = Path(temp) / 'font'
            with self.assertRaises(ValueError): extract(bytes(16777216), out)
            self.assertFalse(out.exists())
