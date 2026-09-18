import unittest
from gba_compress import lz77_decompress, huffman_decompress


class LZ77Tests(unittest.TestCase):
    def test_all_literal(self):
        data = bytes([0x10, 8, 0, 0, 0x00]) + b"ABCDEFGH"
        self.assertEqual(lz77_decompress(data), b"ABCDEFGH")

    def test_back_reference_run(self):
        # flags=0x40: bit7=0 (literal 'A'), bit6=1 (backref length=7 distance=1)
        data = bytes([0x10, 8, 0, 0, 0x40, ord('A'), 0x40, 0x00])
        self.assertEqual(lz77_decompress(data), b"AAAAAAAA")

    def test_rejects_wrong_type(self):
        with self.assertRaises(ValueError):
            lz77_decompress(bytes([0x20, 1, 0, 0]))

    def test_rejects_truncated_stream(self):
        with self.assertRaises(ValueError):
            lz77_decompress(bytes([0x10, 4, 0, 0, 0x00, ord('A')]))

    def test_rejects_backref_before_output_start(self):
        # flags bit7=1 selects a backref as the very first token, with
        # nothing yet in the output for it to reference.
        data = bytes([0x10, 3, 0, 0, 0x80, 0x00, 0x00])
        with self.assertRaises(ValueError):
            lz77_decompress(data)


class HuffmanTests(unittest.TestCase):
    def _two_leaf_tree(self, symbol_a: int, symbol_b: int) -> bytes:
        # tree_size byte=1 -> 4-byte table: [size, root, leafA, leafB];
        # root=0xC0 (both children are leaves, offset 0) puts leafA at
        # child bit 0 and leafB at child bit 1.
        return bytes([1, 0xC0, symbol_a, symbol_b])

    def test_8bit_symbols(self):
        tree = self._two_leaf_tree(0x41, 0x42)
        # bitstream selects leafA (bit=0) then leafB (bit=1), MSB-first
        # in a 32-bit little-endian word: bit31=0, bit30=1.
        stream = (0x40000000).to_bytes(4, "little")
        data = bytes([0x28, 2, 0, 0]) + tree + stream
        self.assertEqual(huffman_decompress(data), b"AB")

    def test_4bit_symbols_pack_into_one_byte(self):
        tree = self._two_leaf_tree(0x03, 0x07)
        stream = (0x40000000).to_bytes(4, "little")
        data = bytes([0x24, 1, 0, 0]) + tree + stream
        self.assertEqual(huffman_decompress(data), bytes([0x73]))

    def test_rejects_wrong_type(self):
        with self.assertRaises(ValueError):
            huffman_decompress(bytes([0x10, 1, 0, 0]))

    def test_rejects_bad_symbol_size(self):
        with self.assertRaises(ValueError):
            huffman_decompress(bytes([0x25, 1, 0, 0]))


if __name__ == "__main__":
    unittest.main()
