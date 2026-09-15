# Agent guardrails

`AGENTS.md` is the canonical contribution contract. This checklist reinforces
the boundaries that matter during M0 bring-up:

- Build a native PS1 runtime; never substitute a GBA emulator.
- Keep PS1 headers, GPU commands and controller services in `src/platform/psx/`.
- Run `python tools/check_host.py` for portable code and tool changes.
- Host compilation does not establish PS1 link, boot or rendering correctness.
- Verify the exact local ROM before extraction. Example manifest offsets are
  placeholders, not an authoritative asset map.
- Keep ROMs, BIOS files and proprietary generated resources out of Git.
- Preserve M10 physical memory-card trading; network/link emulation is excluded.
- Record unavailable inputs and unverified runtime gates in `WORK-HANDOFF.md`.
- Do not mark LGPSX-010+ complete merely because scaffolding or host tests pass.
