"""Pure-Python GBA graphics primitives used by host conversion tools."""
from __future__ import annotations

def bgr555_to_psx(value:int, transparent:bool=False)->int:
    """Convert GBA BGR555 to PS1 BGR555/STP. Bit layout is otherwise compatible."""
    value &= 0x7fff
    if transparent and value == 0: return 0
    return value | 0x8000

def decode_4bpp_tile(data:bytes)->list[int]:
    if len(data)!=32: raise ValueError("4bpp tile must be 32 bytes")
    out=[]
    for b in data: out.extend((b & 0x0f, b >> 4))
    return out

def decode_8bpp_tile(data:bytes)->list[int]:
    if len(data)!=64: raise ValueError("8bpp tile must be 64 bytes")
    return list(data)
