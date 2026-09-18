"""GBA BIOS-compatible decompression (LZ77/LZSS and Huffman).

These are the two general-purpose compression schemes the GBA BIOS exposes
via SWI 0x11 (LZ77UnCompReadNormalWrite8bit/16bit) and SWI 0x13
(HuffUnCompReadNormal). They are hardware-level, general-purpose codecs
used across the entire GBA game library (and documented publicly as part
of the console's BIOS behaviour, e.g. GBATEK) -- not specific to any one
game's copyrighted content. Most compressed GBA ROM data (tiles, tilemaps,
some text) uses one of these two headers, identified by the leading type
nibble in the 4-byte header all BIOS-compressed blocks share.

None of this module embeds any LeafGreen-specific offsets, tables or
data; it only knows how to undo the two console-level compression
formats, given already-verified input bytes a caller supplies.
"""
from __future__ import annotations


def _read_header(data: bytes) -> tuple[int, int]:
    if len(data) < 4:
        raise ValueError("compressed block must have at least a 4-byte header")
    compression_type = data[0] >> 4
    decompressed_size = data[1] | (data[2] << 8) | (data[3] << 16)
    return compression_type, decompressed_size


def lz77_decompress(data: bytes) -> bytes:
    """Decodes a GBA BIOS LZ77/LZSS block (type nibble 0x1)."""
    compression_type, size = _read_header(data)
    if compression_type != 1:
        raise ValueError(f"not an LZ77 block (type nibble {compression_type:#x})")
    out = bytearray()
    pos = 4
    while len(out) < size:
        if pos >= len(data):
            raise ValueError("truncated LZ77 stream (missing flag byte)")
        flags = data[pos]
        pos += 1
        for bit in range(7, -1, -1):
            if len(out) >= size:
                break
            if not (flags >> bit) & 1:
                if pos >= len(data):
                    raise ValueError("truncated LZ77 stream (missing literal byte)")
                out.append(data[pos])
                pos += 1
                continue
            if pos + 1 >= len(data):
                raise ValueError("truncated LZ77 stream (missing back-reference)")
            b0, b1 = data[pos], data[pos + 1]
            pos += 2
            length = (b0 >> 4) + 3
            distance = ((b0 & 0x0f) << 8 | b1) + 1
            if distance > len(out):
                raise ValueError("LZ77 back-reference points before the start of output")
            start = len(out) - distance
            for i in range(length):
                out.append(out[start + i])
    return bytes(out[:size])


def huffman_decompress(data: bytes) -> bytes:
    """Decodes a GBA BIOS Huffman block (type nibble 0x2).

    Supports both the 4-bit and 8-bit symbol-size variants the BIOS
    format allows (data[0] low nibble). Tree node bytes follow the
    documented child-pointer encoding: for a node at (0-based) position
    `pos` within the tree table, its two children sit at
    `(pos & ~1) + (node & 0x3f) * 2 + 2` (bit 0 taken) and that address
    plus one (bit 1 taken); bit 7/6 of the node mark that child as a
    leaf. The root node is the tree table's second byte (index 1),
    right after the tree-size byte (index 0). Bits are read MSB-first
    from 32-bit little-endian words, per the BIOS format.
    """
    compression_type, size = _read_header(data)
    if compression_type != 2:
        raise ValueError(f"not a Huffman block (type nibble {compression_type:#x})")
    symbol_bits = data[0] & 0x0f
    if symbol_bits not in (4, 8):
        raise ValueError(f"unsupported Huffman symbol size: {symbol_bits} bits")

    tree_size_byte = data[4]
    tree_table_len = (tree_size_byte + 1) * 2  # includes the size byte itself
    tree_start = 4
    tree_end = tree_start + tree_table_len
    if tree_end > len(data):
        raise ValueError("truncated Huffman stream (tree)")
    tree = data[tree_start:tree_end]
    stream = data[tree_end:]

    bit_pos = 0

    def read_bit() -> int:
        nonlocal bit_pos
        word_index = bit_pos // 32
        if (word_index + 1) * 4 > len(stream):
            raise ValueError("truncated Huffman bitstream")
        word = int.from_bytes(stream[word_index * 4:word_index * 4 + 4], "little")
        bit = (word >> (31 - (bit_pos % 32))) & 1
        bit_pos += 1
        return bit

    out = bytearray()
    half_nibble = None
    while len(out) < size:
        pos = 1  # root node: tree table index 1 (right after the size byte at index 0)
        while True:
            node = tree[pos]
            bit = read_bit()
            base = (pos & ~1) + (node & 0x3f) * 2 + 2
            child = base + bit
            is_leaf = bool(node & (0x40 if bit else 0x80))
            if child >= len(tree):
                raise ValueError("Huffman tree child index out of range")
            if is_leaf:
                symbol = tree[child]
                break
            pos = child
        if symbol_bits == 8:
            out.append(symbol)
        elif half_nibble is None:
            half_nibble = symbol & 0x0f
        else:
            out.append(half_nibble | ((symbol & 0x0f) << 4))
            half_nibble = None
    return bytes(out[:size])
