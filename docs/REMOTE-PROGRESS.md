# Remote progress snapshot

## Completed without interactive Work/runtime
- M0 foundation LGPSX-001..007
- LGPSX-008 versioned manifest + SHA-1-gated local extraction boundary
- LGPSX-009 deterministic GBA 4bpp/palette -> PS1 TIM prototype
- host CI and strict C compilation for portable modules
- text renderer API/design scaffold for LGPSX-010
- platform-neutral overworld movement/collision scaffold for later M0 tasks
- logical resource/area-bundle index boundary for CD-oriented loading
- bounded yielding script VM scaffold for LGPSX-019
- versioned logical save envelope separated from memory-card transport
- PS1 memory budget, acceptance gate and testing strategy
- formal M0-M10 roadmap, decisions and porting workflow
- future memory-card LGTR envelope, CRC validation and host test
- agent guardrails and local/remote handoff documentation

## Hard blockers before claiming later M0 tasks complete
LGPSX-010 onward needs real locally generated LeafGreen assets and/or PS1 runtime verification. Portable scaffolding can continue remotely, but render/map tasks must not be marked complete without seeing exact generated resources execute in the PS1 backend.

The next high-value interactive step is to run the verified ROM/extraction pipeline, build with PSn00bSDK, and boot the current target. Runtime findings should then feed the next repository iteration.
