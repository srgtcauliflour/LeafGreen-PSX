#!/usr/bin/env python3
"""Manifest-driven charmap text extraction from a verified local ROM.

Same shape as tools/assets/extract.py (verify the whole ROM's SHA-1,
then only touch explicit offset/length ranges from a manifest), but
decodes each range through tools/textconv/charmap.py instead of copying
raw bytes -- for "text"-kind manifest entries specifically. This tool
does not know or publish any real LeafGreen text offsets itself.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from charmap import decode_text

EXPECTED_SHA1 = "7862c67bdecbe21d1d69ce082ce34327e1c6ed5e"


def sha1(path: Path) -> str:
    digest = hashlib.sha1()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--strict", action="store_true",
                         help="fail on any byte with no known glyph/control mapping "
                              "instead of emitting a \\xNN placeholder")
    args = parser.parse_args()

    if sha1(args.rom) != EXPECTED_SHA1:
        parser.error("ROM is not the supported LeafGreen USA Rev 1 target")

    doc = json.loads(args.manifest.read_text(encoding="utf-8"))
    if doc.get("schema") != 1 or doc.get("target") != "leafgreen-us-rev1":
        parser.error("unsupported manifest")

    size = args.rom.stat().st_size
    on_unmapped = "strict" if args.strict else "placeholder"
    with args.rom.open("rb") as stream:
        for item in doc.get("assets", []):
            if item.get("kind") != "text":
                continue
            source = item.get("source", {})
            offset, length = source.get("offset"), source.get("length")
            if not isinstance(offset, int) or not isinstance(length, int) or \
                    offset < 0 or length < 0 or offset + length > size:
                parser.error(f"invalid source range for {item.get('id')}")
            stream.seek(offset)
            try:
                decoded = decode_text(stream.read(length), on_unmapped=on_unmapped)
            except ValueError as exc:
                parser.error(f"{item.get('id')}: {exc}")
            out = Path(item["output"])
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_text(decoded, encoding="utf-8")
            print(f"{item['id']}: {out} ({len(decoded)} chars)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
