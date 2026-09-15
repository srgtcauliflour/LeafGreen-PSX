# PS1 memory budget

This is an engineering budget, not a claim that every category will use its full allowance. Actual values must be measured at LGPSX-020.

## Main RAM — 2 MiB target envelope

| Category | Initial ceiling |
|---|---:|
| executable + static engine state | 640 KiB |
| current area/map/scripts/object state | 256 KiB |
| graphics staging/decompression | 256 KiB |
| gameplay heap/temporary work | 320 KiB |
| audio/CD streaming buffers | 192 KiB |
| stack/system/SDK/reserve | 384 KiB |

The ceilings sum to the hardware envelope and force explicit trade-offs. Large mutually exclusive systems should become overlays or CD-loaded modules rather than permanently resident code/data.

## VRAM

Treat PS1 VRAM as a separately budgeted resource. Reserve framebuffer/display regions first, then allocate texture pages and CLUTs deterministically. Indexed 4bpp assets are preferred where LeafGreen source art permits them. The converter must eventually emit placement metadata rather than allowing ad-hoc runtime allocation.

## SPU RAM

Music/SFX assets must be streamed/loaded by context; the complete soundtrack must never be assumed resident. Audio receives its own later budget once the overworld vertical slice is stable.

## Area bundles

Pallet Town M0 should prove an area-bundle strategy: current maps, local NPC graphics, scripts and required UI are grouped for predictable CD reads. Battle and other mutually exclusive modes can later replace area-specific working data.

## Measurement gates

At LGPSX-020 record PS-EXE size, static data, peak heap, stack headroom, VRAM map, largest CD read, load latency and frame time. No subsystem is considered optimized solely because it fits on an emulator.
