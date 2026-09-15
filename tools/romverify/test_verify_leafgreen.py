#!/usr/bin/env python3
"""Unit tests for the LeafGreen Rev 1 ROM verifier constants/header checks.

These tests use synthetic bytes only; no copyrighted ROM data is stored here.
"""

import unittest

import verify_leafgreen as verifier


class LeafGreenVerifierTests(unittest.TestCase):
    def test_expected_target_metadata(self):
        self.assertEqual(verifier.EXPECTED_SIZE, 16 * 1024 * 1024)
        self.assertEqual(verifier.EXPECTED_GAME_CODE, b"BPGE")
        self.assertEqual(verifier.EXPECTED_REVISION, 1)
        self.assertEqual(verifier.EXPECTED_SHA1, "7862c67bdecbe21d1d69ce082ce34327e1c6ed5e")

    def test_gba_header_offsets(self):
        header = bytearray(0xBD)
        header[0xAC:0xB0] = b"BPGE"
        header[0xBC] = 1
        self.assertEqual(bytes(header[0xAC:0xB0]), verifier.EXPECTED_GAME_CODE)
        self.assertEqual(header[0xBC], verifier.EXPECTED_REVISION)


if __name__ == "__main__":
    unittest.main()
