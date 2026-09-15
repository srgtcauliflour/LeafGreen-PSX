#!/usr/bin/env python3
"""Unit tests for the LeafGreen Rev 1 ROM verifier constants/header checks.

These tests use synthetic bytes only; no copyrighted ROM data is stored here.
"""

import hashlib
import unittest

import verify_leafgreen as verifier


class LeafGreenVerifierTests(unittest.TestCase):
    def test_expected_target_metadata(self):
        self.assertEqual(verifier.EXPECTED_SIZE, 16 * 1024 * 1024)
        self.assertEqual(verifier.EXPECTED_GAME_CODE, b"BPGE")
        self.assertEqual(verifier.EXPECTED_REVISION, 1)
        self.assertEqual(len(verifier.EXPECTED_SHA1), 40)

    def test_gba_header_offsets(self):
        header = bytearray(0xBD)
        header[0xAC:0xB0] = b"BPGE"
        header[0xBC] = 1
        self.assertEqual(bytes(header[0xAC:0xB0]), verifier.EXPECTED_GAME_CODE)
        self.assertEqual(header[0xBC], verifier.EXPECTED_REVISION)

    def test_sha1_algorithm_is_stable(self):
        self.assertEqual(
            hashlib.sha1(b"LeafGreen-PSX").hexdigest(),
            "66ef86f3955e48eb23d9d9d107ded5dc0d79e84b",
        )


if __name__ == "__main__":
    unittest.main()
