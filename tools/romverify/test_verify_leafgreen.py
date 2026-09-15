#!/usr/bin/env python3
"""Unit tests for the LeafGreen Rev 1 ROM verifier.

These tests use synthetic bytes only; no copyrighted ROM data is stored here.
"""

from pathlib import Path
import tempfile
import unittest

import verify_leafgreen as verifier


class LeafGreenVerifierTests(unittest.TestCase):
    def test_expected_target_metadata(self):
        self.assertEqual(verifier.EXPECTED_SIZE, 16 * 1024 * 1024)
        self.assertEqual(verifier.EXPECTED_GAME_CODE, b"BPGE")
        self.assertEqual(verifier.EXPECTED_REVISION, 1)
        self.assertEqual(verifier.EXPECTED_SHA1, "7862c67bdecbe21d1d69ce082ce34327e1c6ed5e")

    def test_reads_gba_header_fields(self):
        header = bytearray(0xBD)
        header[0xAC:0xB0] = b"BPGE"
        header[0xBC] = 1
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "synthetic.gba"
            path.write_bytes(header)
            game_code, revision = verifier.read_target_header(path)
        self.assertEqual(game_code, verifier.EXPECTED_GAME_CODE)
        self.assertEqual(revision, verifier.EXPECTED_REVISION)

    def test_streamed_sha1(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "synthetic.bin"
            path.write_bytes(b"abc")
            digest = verifier.sha1_file(path)
        self.assertEqual(digest, "a9993e364706816aba3e25717850c26c9cd0d89d")


if __name__ == "__main__":
    unittest.main()
