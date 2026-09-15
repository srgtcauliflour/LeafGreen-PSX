#!/usr/bin/env python3
"""Linux headless PS1 smoke run using an externally built libretro core."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--core', required=True, type=Path)
    parser.add_argument('--headers', required=True, type=Path,
                        help='Directory containing the matching libretro.h')
    parser.add_argument('--nm', required=True, type=Path, help='PS1 toolchain nm')
    parser.add_argument('--elf', type=Path, default=ROOT / 'build/leafgreen_psx.elf')
    parser.add_argument('--cue', type=Path, default=ROOT / 'build/leafgreen_psx.cue')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/runtime-smoke')
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error('Output must be empty (it also serves as the BIOS/save directory)')
    symbols = {}
    for line in subprocess.check_output([str(args.nm.resolve()), str(args.elf.resolve())], text=True).splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] in ('s_frame_counter', 's_previous_buttons'):
            symbols[parts[2]] = parts[0]
    if len(symbols) != 2:
        parser.error('Expected diagnostic symbols missing from ELF; use a matching debug build')
    output.mkdir(parents=True, exist_ok=True)
    core = args.core.resolve()
    with tempfile.TemporaryDirectory(prefix='lgpsx-runtime-') as temporary:
        binary = Path(temporary) / 'smoke'
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        '-I' + str(args.headers.resolve()),
                        str(ROOT / 'tools/runtime/libretro_smoke.c'), str(core),
                        '-Wl,-rpath,' + str(core.parent), '-o', str(binary)], check=True)
        try:
            result = subprocess.run([str(binary), str(args.cue.resolve()), str(output),
                                     symbols['s_frame_counter'], symbols['s_previous_buttons']],
                                    timeout=45, capture_output=True, text=True)
        except subprocess.TimeoutExpired as exc:
            (output / 'timeout.txt').write_text('Core exceeded 45-second limit. Runtime gate failed.\n')
            raise SystemExit('FAIL: emulator timed out') from exc
    (output / 'run.log').write_text(result.stdout + result.stderr)
    captures = sorted(output.glob('frame-*.ppm'))
    (output / 'report.json').write_text(json.dumps({
        'exit_code': result.returncode, 'bios': 'HLE (empty system directory)',
        'elf_sha256': sha256(args.elf), 'core_sha256': sha256(core),
        'symbols': symbols,
        'captures': {path.name: sha256(path) for path in captures},
        'rendering': 'Manual inspection required; hashes alone do not prove correctness',
    }, indent=2) + '\n')
    print(result.stdout, end='')
    if result.stderr:
        print(result.stderr, end='')
    if result.returncode:
        raise SystemExit(result.returncode)


if __name__ == '__main__':
    main()
