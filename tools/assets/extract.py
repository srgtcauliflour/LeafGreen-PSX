#!/usr/bin/env python3
"""Manifest-driven byte extraction from a verified local ROM.

Only explicit offset/length descriptors are supported in v1. Generated outputs
must remain ignored. This tool does not know or publish proprietary offsets.
"""
from __future__ import annotations
import argparse, hashlib, json
from pathlib import Path
EXPECTED_SHA1="7862c67bdecbe21d1d69ce082ce34327e1c6ed5e"
def sha1(path:Path)->str:
 h=hashlib.sha1()
 with path.open("rb") as f:
  for chunk in iter(lambda:f.read(1024*1024),b""):h.update(chunk)
 return h.hexdigest()
def main()->int:
 p=argparse.ArgumentParser();p.add_argument("rom",type=Path);p.add_argument("manifest",type=Path);a=p.parse_args()
 if sha1(a.rom)!=EXPECTED_SHA1:p.error("ROM is not supported LeafGreen USA Rev 1")
 doc=json.loads(a.manifest.read_text())
 if doc.get("schema")!=1 or doc.get("target")!="leafgreen-us-rev1":p.error("unsupported manifest")
 size=a.rom.stat().st_size
 with a.rom.open("rb") as f:
  for item in doc.get("assets",[]):
   src=item.get("source",{});off=src.get("offset");length=src.get("length")
   if not isinstance(off,int) or not isinstance(length,int) or off<0 or length<0 or off+length>size:p.error(f"invalid source range for {item.get('id')}")
   out=Path(item["output"]);out.parent.mkdir(parents=True,exist_ok=True);f.seek(off);out.write_bytes(f.read(length));print(f"{item['id']}: {out}")
 return 0
if __name__=="__main__":raise SystemExit(main())
