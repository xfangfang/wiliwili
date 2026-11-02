#!/usr/bin/env bash
# wiliwili - PS5 native title: replay the CI job "build-ps5-title" on this
# machine, step by step, so a workflow run is not the first time a change is
# tried.
#
#   scripts/ps5/title/verify_local.sh            # everything, about 20 minutes
#   VERIFY_STEPS=3 scripts/ps5/title/verify_local.sh   # stop after configure
#
# It uses its own build and output directories, so the folder that is deployed
# from dist/ is left alone.

set -euo pipefail

root=$(cd -- "$(dirname -- "$0")/../../.." && pwd)
build=${PS5_VERIFY_BUILD_DIR:-$root/cmake-build-ps5-citest}
out=${PS5_VERIFY_OUT_DIR:-$root/dist-citest/PPSA99099}
steps=${VERIFY_STEPS:-5}

cd "$root"

step() { printf '\n===== %s =====\n' "$*"; }

source scripts/ps5/title/env.sh > /dev/null
# The CI job exports the compiler of the llvm-18 package the same way; a
# distribution's clang usually works here too, but the host tools are built
# with the same LLVM the rest of the pipeline uses.
export CC="$PS5_CLANG"
export CXX="$(dirname -- "$PS5_CLANG")/clang++"

step "1/$steps resource generator"
rm -rf library/borealis/build_libromfs_generator
bash library/borealis/build_libromfs_generator.sh

step "2/$steps runtime module"
for attempt in 1 2 3; do
    if make -C "$PS5_BOILERPLATE" libc USE_CCACHE=0 \
        HOST_CC="$CC" HOST_CXX="$CXX"; then
        break
    fi
    echo "attempt $attempt failed, dropping the downloaded zlib" >&2
    rm -rf "$PS5_BOILERPLATE/.deps/native/zlib"
    [[ $attempt == 3 ]] && exit 1
done

step "3/$steps configure"
rm -rf "$build"
cmake -B "$build" -DCMAKE_BUILD_TYPE=Release -DPLATFORM_PS5=ON \
    -DPS5_NATIVE_TITLE=ON -DUSE_LIBROMFS=ON -DBRLS_UNITY_BUILD=ON \
    -DCMAKE_UNITY_BUILD_BATCH_SIZE=16 2>&1 | tail -5

if [[ $steps -lt 4 ]]; then
    echo "stopping after configure"
    exit 0
fi

step "4/$steps build and assemble"
PS5_TITLE_OUT_DIR="$out" cmake --build "$build" --target wiliwili.title -j4 2>&1 | tail -25

step "5/$steps result"
ls -l "$out" "$out/sce_module" "$out/sce_sys"
sha256_of() { sha256sum "$1" 2> /dev/null || shasum -a 256 "$1"; }
sha256_of "$out/eboot.bin"
sha256_of "$out/sce_module/libc.prx"
echo "embedded resources: $(strings "$out/eboot.bin" | grep -c port_header) port_header"
