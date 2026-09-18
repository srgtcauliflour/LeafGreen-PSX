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
    for suite in ('romverify', 'assets', 'gfxconv', 'font'):
        env['PYTHONPATH'] = str(ROOT / 'tools' / suite)
        run([sys.executable, '-m', 'unittest', 'discover', '-s',
             'tools/' + suite, '-p', 'test_*.py'], env=env)
    run([sys.executable, 'tools/assets/manifest.py',
         'tools/assets/leafgreen_rev1.example.json'], env=env)
    compiler = shlex.split(os.environ.get('CC', 'cc'))
    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-Iinclude']
    modules = {'text': ['src/game/text.c'],
               'dialogue': ['src/game/dialogue.c'],
               'window': ['src/game/window.c', 'src/game/dialogue.c'],
               'overworld': ['src/overworld/overworld.c'],
               'script': ['src/script/vm.c'],
               'service': ['src/game/service.c', 'src/script/vm.c', 'src/overworld/overworld.c'],
               'loop': ['src/game/loop.c', 'src/game/service.c', 'src/game/window.c',
                        'src/game/dialogue.c', 'src/script/vm.c', 'src/overworld/overworld.c'],
               'trade_record': ['src/game/trade_record.c'],
               'save': ['src/game/save.c'],
               'resource': ['src/game/resource.c']}
    all_sources = sorted({source for sources in modules.values() for source in sources})
    with tempfile.TemporaryDirectory(prefix='lgpsx-host-') as temp:
        for name, sources in modules.items():
            binary = Path(temp) / ('test_' + name + '.exe')
            run(compiler + flags + [*sources, 'tests/host/test_' + name + '.c',
                                    '-o', str(binary)])
            run([str(binary)])
        for source in ['src/main.c', *all_sources]:
            run(compiler + flags + ['-c', source, '-o',
                                    str(Path(temp) / (Path(source).stem + '.o'))])
    print('PASS: all host checks (PS1 backend/runtime not validated).')


if __name__ == '__main__':
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as exc:
        print('Host checks failed: ' + str(exc), file=sys.stderr)
        sys.exit(1)
