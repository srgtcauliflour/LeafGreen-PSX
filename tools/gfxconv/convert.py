#!/usr/bin/env python3
"""Small deterministic GBA indexed-graphics -> PS1 TIM converter."""
from __future__ import annotations
import argparse, struct, sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from gba import bgr555_to_psx
from tim import tim4

def main()->int:
    p=argparse.ArgumentParser(); p.add_argument("tiles",type=Path); p.add_argument("palette",type=Path); p.add_argument("output",type=Path); a=p.parse_args()
    pixels=a.tiles.read_bytes(); raw=a.palette.read_bytes()
    if len(raw)!=32: p.error("palette must contain exactly 16 little-endian BGR555 colours")
    pal=[bgr555_to_psx(v,i==0) for i,v in enumerate(struct.unpack("<16H",raw))]
    if len(pixels)%32: p.error("4bpp tile data must be a multiple of 32 bytes")
    # Prototype supports a horizontal strip of tiles; enough to validate PS1 upload/render path.
    count=len(pixels)//32; width=8*count; height=8
    packed=bytearray((width*height)//2)
    for t in range(count):
        tile=pixels[t*32:(t+1)*32]
        for y in range(8): packed[(y*width//2)+(t*4):(y*width//2)+(t*4)+4]=tile[y*4:y*4+4]
    a.output.parent.mkdir(parents=True,exist_ok=True); a.output.write_bytes(tim4(bytes(packed),pal,width,height)); print(a.output); return 0
if __name__=="__main__": raise SystemExit(main())
