#!/usr/bin/env python3
"""Validate LeafGreen-PSX asset manifests without shipping game assets."""
from __future__ import annotations
import argparse, json
from pathlib import Path

REQUIRED={"schema","target","assets"}
KINDS={"palette","tiles4","tiles8","tilemap","sprite4","sprite8","font","script","map","audio","text"}

def validate(doc: dict) -> list[str]:
    errors=[]
    missing=REQUIRED-set(doc)
    if missing: errors.append("missing top-level keys: "+", ".join(sorted(missing)))
    if doc.get("schema") != 1: errors.append("schema must be 1")
    if doc.get("target") != "leafgreen-us-rev1": errors.append("unsupported target")
    assets=doc.get("assets",[])
    if not isinstance(assets,list): return errors+["assets must be a list"]
    ids=set()
    for i,a in enumerate(assets):
        if not isinstance(a,dict): errors.append(f"assets[{i}] must be an object"); continue
        aid=a.get("id")
        if not isinstance(aid,str) or not aid: errors.append(f"assets[{i}].id missing")
        elif aid in ids: errors.append(f"duplicate asset id: {aid}")
        else: ids.add(aid)
        if a.get("kind") not in KINDS: errors.append(f"assets[{i}].kind unsupported")
        if "source" not in a: errors.append(f"assets[{i}].source missing")
        if "output" not in a: errors.append(f"assets[{i}].output missing")
    return errors

def main()->int:
    p=argparse.ArgumentParser(); p.add_argument("manifest",type=Path); a=p.parse_args()
    doc=json.loads(a.manifest.read_text(encoding="utf-8")); errors=validate(doc)
    if errors:
        print("FAIL:"); [print(" -",e) for e in errors]; return 1
    print(f"PASS: {len(doc['assets'])} asset entries validated."); return 0
if __name__=="__main__": raise SystemExit(main())
