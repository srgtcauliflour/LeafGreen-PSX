#!/usr/bin/env python3
"""Verify a locally supplied Pokemon LeafGreen US Rev 1 ROM.

The ROM is never copied into the repository or build output.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import sys

EXPECTED_SIZE = 16 * 1024 * 1024
EXPECTED_SHA1 = "7862c67bdecbe21d1d69ce082ce34327e1c6ed5e"
EXPECTED_GAME_CODE = b"BPGE"
EXPECTED_REVISION = 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path, help="Path to LeafGreen US v1.1 .gba")
    args = parser.parse_args()

    data = args.rom.read_bytes()
    sha1 = hashlib.sha1(data).hexdigest()
    game_code = data[0xAC:0xB0] if len(data) >= 0xBD else b""
    revision = data[0xBC] if len(data) >= 0xBD else -1

    checks = {
        "size": len(data) == EXPECTED_SIZE,
        "sha1": sha1 == EXPECTED_SHA1,
        "game_code": game_code == EXPECTED_GAME_CODE,
        "revision": revision == EXPECTED_REVISION,
    }

    print(f"ROM:       {args.rom}")
    print(f"Size:      {len(data)} bytes")
    print(f"SHA-1:     {sha1}")
    print(f"Game code: {game_code.decode('ascii', errors='replace')}")
    print(f"Revision:  {revision}")

    if all(checks.values()):
        print("PASS: Pokemon LeafGreen (USA) Rev 1 target verified.")
        return 0

    print("FAIL: ROM does not match the supported LeafGreen Rev 1 target.", file=sys.stderr)
    for name, passed in checks.items():
        if not passed:
            print(f"  mismatch: {name}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
