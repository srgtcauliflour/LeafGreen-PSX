#!/bin/bash
# Provisions the mipsel-none-elf/PSn00bSDK PS1 toolchain for Claude Code on
# the web sessions, so the real PS1 CMake target (psn00bsdk_add_executable
# in CMakeLists.txt) is buildable, not just tools/check_host.py's host-only
# scaffolding. Building GCC/binutils from source can take 30-60 minutes, so
# this runs async; tools/psn00bsdk/setup.sh is idempotent and skips any
# stage whose output already exists, so re-running (including on session
# resume) after the first successful install is fast.
set -euo pipefail

echo '{"async": true, "asyncTimeout": 3600000}'

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
    exit 0
fi

bash "$CLAUDE_PROJECT_DIR/tools/psn00bsdk/setup.sh"

if [ -n "${CLAUDE_ENV_FILE:-}" ]; then
    echo 'export PATH="/opt/mipsel-none-elf/bin:$PATH"' >> "$CLAUDE_ENV_FILE"
    echo 'export PSN00BSDK_LIBS="/opt/psn00bsdk/lib/libpsn00b"' >> "$CLAUDE_ENV_FILE"
fi
