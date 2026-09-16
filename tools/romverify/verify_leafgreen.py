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


def sha1_file(path: Path) -> str:
    digest = hashlib.sha1()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_target_header(path: Path) -> tuple[bytes, int]:
    with path.open("rb") as stream:
        stream.seek(0xAC)
        game_code = stream.read(4)
        stream.seek(0xBC)
        revision_raw = stream.read(1)
    return game_code, revision_raw[0] if revision_raw else -1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path, help="Path to LeafGreen US v1.1 .gba")
    args = parser.parse_args()

    if not args.rom.is_file():
        print(f"FAIL: ROM file not found: {args.rom}", file=sys.stderr)
        return 2

    size = args.rom.stat().st_size
    sha1 = sha1_file(args.rom)
    game_code, revision = read_target_header(args.rom)

    checks = {
        "size": size == EXPECTED_SIZE,
        "sha1": sha1 == EXPECTED_SHA1,
        "game_code": game_code == EXPECTED_GAME_CODE,
        "revision": revision == EXPECTED_REVISION,
    }

    print(f"ROM:       {args.rom}")
    print(f"Size:      {size} bytes")
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
