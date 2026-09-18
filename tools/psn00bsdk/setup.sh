#!/usr/bin/env bash
# Installs (idempotently) a mipsel-none-elf bare-metal GCC toolchain and
# PSn00bSDK, so LeafGreen-PSX's real PS1 CMake target
# (psn00bsdk_add_executable in CMakeLists.txt) can actually be built and
# linked, not just the host-only tools/check_host.py scaffolding.
#
# Ubuntu's packaged mipsel-linux-gnu cross-compiler is NOT sufficient:
# PSn00bSDK explicitly does not support mipsel-linux-gnu toolchains (see
# PSn00bSDK doc/cmake_reference.md), only bare-metal mipsel-none-elf or
# mipsel-unknown-elf. This script builds one from source per
# doc/toolchain.md, since prebuilt GitHub release archives are not
# reachable from this environment's network policy.
#
# Safe to re-run: each stage is skipped if its output already exists.
set -euo pipefail

TOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX:-/opt/mipsel-none-elf}"
PSN00BSDK_SRC="${PSN00BSDK_SRC:-/opt/psn00bsdk-src}"
PSN00BSDK_PREFIX="${PSN00BSDK_PREFIX:-/opt/psn00bsdk}"
BUILD_ROOT="${BUILD_ROOT:-/opt/psn00bsdk-toolchain-build}"
BINUTILS_VERSION="2.40"
GCC_VERSION="12.2.0"
JOBS="${JOBS:-$(nproc)}"
PROFILE_SCRIPT="/etc/profile.d/psn00bsdk.sh"

log() { echo "[psn00bsdk-setup] $*"; }

# ftpmirror.gnu.org (and similar mirror-redirect services) occasionally
# hands back a transient error (wget exit 8) on the very first request --
# retry with backoff instead of failing the whole build on one flaky
# redirect. Downloads to a .partial path first so a failed/interrupted
# attempt never leaves a corrupt file at the final name.
download_with_retry() {
    local url="$1" out="$2" attempt
    if [ -f "$out" ]; then return 0; fi
    for attempt in 1 2 3 4 5; do
        if wget -q --tries=3 --timeout=30 -O "${out}.partial" "$url"; then
            mv "${out}.partial" "$out"
            return 0
        fi
        rm -f "${out}.partial"
        log "download attempt ${attempt}/5 failed for ${url}, retrying in $((attempt * 5))s..."
        sleep $((attempt * 5))
    done
    log "FATAL: failed to download ${url} after 5 attempts"
    return 1
}

# Same idea for a command that does its own downloading internally (GCC's
# contrib/download_prerequisites fetches gmp/mpfr/mpc/isl) and can't be
# retried file-by-file.
retry_command() {
    local attempt
    for attempt in 1 2 3; do
        if "$@"; then return 0; fi
        log "command failed (attempt ${attempt}/3): $*"
        sleep $((attempt * 5))
    done
    return 1
}

export PATH="$TOOLCHAIN_PREFIX/bin:$PATH"

# --- Stage 1: host build prerequisites -------------------------------------
# Always ensure these (apt is a fast no-op for anything already installed) --
# a partial "build-essential is already here" host must not skip texinfo/
# bison/flex, which binutils'/GCC's build still needs even with
# --disable-docs (bfd's own doc/ subdir isn't covered by that flag).
log "Ensuring host build prerequisites..."
apt-get update -qq
apt-get install -y -qq build-essential wget cmake ninja-build git \
    lua5.4 liblua5.4-dev texinfo bison flex

# --- Stage 2: mipsel-none-elf binutils --------------------------------------
if command -v "${TOOLCHAIN_PREFIX}/bin/mipsel-none-elf-ld" >/dev/null 2>&1; then
    log "binutils already installed at ${TOOLCHAIN_PREFIX}, skipping."
else
    log "Building binutils ${BINUTILS_VERSION} for mipsel-none-elf..."
    mkdir -p "$BUILD_ROOT"
    cd "$BUILD_ROOT"
    download_with_retry \
        "https://ftpmirror.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz" \
        "binutils-${BINUTILS_VERSION}.tar.xz"
    [ -d "binutils-${BINUTILS_VERSION}" ] || tar xf "binutils-${BINUTILS_VERSION}.tar.xz"
    rm -rf binutils-build
    mkdir binutils-build
    cd binutils-build
    ../"binutils-${BINUTILS_VERSION}"/configure \
        --prefix="$TOOLCHAIN_PREFIX" --target=mipsel-none-elf \
        --disable-docs --disable-nls --disable-werror --with-float=soft
    make -j "$JOBS"
    make install-strip
fi

# --- Stage 3: mipsel-none-elf GCC -------------------------------------------
if command -v "${TOOLCHAIN_PREFIX}/bin/mipsel-none-elf-gcc" >/dev/null 2>&1; then
    log "GCC already installed at ${TOOLCHAIN_PREFIX}, skipping."
else
    log "Building GCC ${GCC_VERSION} for mipsel-none-elf (this takes a while)..."
    cd "$BUILD_ROOT"
    download_with_retry \
        "https://ftpmirror.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz" \
        "gcc-${GCC_VERSION}.tar.xz"
    [ -d "gcc-${GCC_VERSION}" ] || tar xf "gcc-${GCC_VERSION}.tar.xz"
    cd "gcc-${GCC_VERSION}"
    retry_command ./contrib/download_prerequisites
    cd "$BUILD_ROOT"
    rm -rf gcc-build
    mkdir gcc-build
    cd gcc-build
    ../"gcc-${GCC_VERSION}"/configure \
        --prefix="$TOOLCHAIN_PREFIX" --target=mipsel-none-elf \
        --disable-docs --disable-nls --disable-werror --disable-libada \
        --disable-libssp --disable-libquadmath --disable-threads \
        --disable-libgomp --disable-libstdcxx-pch --disable-hosted-libstdcxx \
        --enable-languages=c,c++ --without-isl --without-headers \
        --with-float=soft --with-gnu-as --with-gnu-ld
    make -j "$JOBS"
    make install-strip
fi

# --- Stage 4: PSn00bSDK itself -----------------------------------------------
if [ -f "${PSN00BSDK_PREFIX}/lib/libpsn00b/cmake/sdk.cmake" ]; then
    log "PSn00bSDK already installed at ${PSN00BSDK_PREFIX}, skipping."
else
    log "Building PSn00bSDK..."
    if [ ! -d "$PSN00BSDK_SRC/.git" ]; then
        retry_command git clone --recurse-submodules --depth 1 \
            https://github.com/Lameguy64/PSn00bSDK.git "$PSN00BSDK_SRC"
    fi
    cd "$PSN00BSDK_SRC"
    cmake -S . -B build -G Ninja \
        -DCMAKE_INSTALL_PREFIX="$PSN00BSDK_PREFIX" \
        -DPSN00BSDK_TARGET=mipsel-none-elf \
        -DPSN00BSDK_TC="$TOOLCHAIN_PREFIX"
    cmake --build build
    cmake --install build
fi

# --- Stage 5: persistent environment ----------------------------------------
cat > "$PROFILE_SCRIPT" <<EOF
export PATH="$TOOLCHAIN_PREFIX/bin:\$PATH"
export PSN00BSDK_LIBS="$PSN00BSDK_PREFIX/lib/libpsn00b"
EOF
log "Wrote $PROFILE_SCRIPT"

log "Done. mipsel-none-elf-gcc: $(command -v mipsel-none-elf-gcc || echo MISSING)"
log "PSn00bSDK sdk.cmake: ${PSN00BSDK_PREFIX}/lib/libpsn00b/cmake/sdk.cmake"
