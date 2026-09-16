"""Minimal PS1 TIM writer for indexed 4bpp assets."""
from __future__ import annotations
import struct

def tim4(tile_data:bytes, palette:list[int], width:int=8, height:int=8)->bytes:
    if width % 4: raise ValueError("4bpp TIM width must be divisible by 4 pixels")
    if len(tile_data)!=(width*height)//2: raise ValueError("pixel payload size mismatch")
    if len(palette)!=16: raise ValueError("4bpp TIM requires 16 palette entries")
    # TIM: magic, flags(CLUT+4bpp), CLUT block, image block. Width fields use 16-bit VRAM words.
    clut=struct.pack("<IHHHH",12+32,0,0,16,1)+struct.pack("<16H",*palette)
    image=struct.pack("<IHHHH",12+len(tile_data),0,0,width//4,height)+tile_data
    return struct.pack("<II",0x10,0x08)+clut+image
