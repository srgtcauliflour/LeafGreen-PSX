#!/usr/bin/env python3
"""Run all asset-free host checks from any working directory."""
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(args, **kwargs):
    print('+ ' + shlex.join(map(str, args)), flush=True)
    subprocess.run(args, cwd=ROOT, check=True, **kwargs)


def main():
    env = os.environ.copy()
    env['PYTHONDONTWRITEBYTECODE'] = '1'
    for suite in ('romverify', 'assets', 'gfxconv'):
        env['PYTHONPATH'] = str(ROOT / 'tools' / suite)
        run([sys.executable, '-m', 'unittest', 'discover', '-s',
             'tools/' + suite, '-p', 'test_*.py'], env=env)
    run([sys.executable, 'tools/assets/manifest.py',
         'tools/assets/leafgreen_rev1.example.json'], env=env)
    compiler = shlex.split(os.environ.get('CC', 'cc'))
    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-Iinclude']
    modules = {'overworld': 'src/overworld/overworld.c',
               'script': 'src/script/vm.c',
               'trade_record': 'src/game/trade_record.c',
               'save': 'src/game/save.c',
               'resource': 'src/game/resource.c'}
    with tempfile.TemporaryDirectory(prefix='lgpsx-host-') as temp:
        for name, source in modules.items():
            binary = Path(temp) / ('test_' + name + '.exe')
            run(compiler + flags + [source, 'tests/host/test_' + name + '.c',
                                    '-o', str(binary)])
            run([str(binary)])
        for source in ['src/main.c', 'src/game/text.c', *modules.values()]:
            run(compiler + flags + ['-c', source, '-o',
                                    str(Path(temp) / (Path(source).stem + '.o'))])
    print('PASS: all host checks (PS1 backend/runtime not validated).')


if __name__ == '__main__':
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as exc:
        print('Host checks failed: ' + str(exc), file=sys.stderr)
        sys.exit(1)
