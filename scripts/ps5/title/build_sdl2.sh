#!/usr/bin/env bash
# wiliwili - PS5 native title: build the static SDL2 the title links.
#
# A title links everything into one image, so SDL2 has to be a static archive.
# The console's OpenGL implementation ships an SDL2 integration for exactly that
# (video output, the DualSense pad and the audio output), and this script drives
# its own builder with the small patch in patches/ps5-opengl-sdl2-static.patch,
# which is what turns the audio subsystem on.
#
# Required environment:
#   PS5_OPENGL_CHECKOUT   ps5-opengl source checkout (scripts/ps5/title/fetch_deps.sh)
#   PS5_OPENGL_PREFIX     ps5-opengl SDK prefix, the builder verifies its manifest
#   PS5_PAYLOAD_SDK       ps5-payload-sdk (the cross compiler)
#   PS5_BOILERPLATE       ps5-native-app-boilerplate checkout (the compiler wrapper)
#   PS5_SDL_SOURCE        SDL2 git checkout at the pinned commit
#   PS5_CLANG             host clang of the LLVM installation to take llvm-ar from
# Optional:
#   PS5_SDL2_PREFIX       where the prefix is installed (default: the ps5-opengl
#                         build directory, build/sdl2-native/sdk)
#   PS5_SDL2_FORCE=1      rebuild even when the prefix is already there

set -euo pipefail

: "${PS5_OPENGL_CHECKOUT:?set PS5_OPENGL_CHECKOUT to a ps5-opengl source checkout}"
: "${PS5_OPENGL_PREFIX:?set PS5_OPENGL_PREFIX to the ps5-opengl SDK prefix}"
: "${PS5_PAYLOAD_SDK:?set PS5_PAYLOAD_SDK to a ps5-payload-sdk installation}"
: "${PS5_BOILERPLATE:?set PS5_BOILERPLATE to a ps5-native-app-boilerplate checkout}"
: "${PS5_SDL_SOURCE:?set PS5_SDL_SOURCE to an SDL2 git checkout}"

integration=$PS5_OPENGL_CHECKOUT/integration/SDL2
patch_file=$(cd "$(dirname "$0")" && pwd)/patches/ps5-opengl-sdl2-static.patch
native_build=$PS5_OPENGL_CHECKOUT/build/sdl2-native
sdl2_prefix=${PS5_SDL2_PREFIX:-$native_build/sdk}

[[ -d $integration ]] || {
    echo "$integration is missing; PS5_OPENGL_CHECKOUT is not a ps5-opengl checkout" >&2
    exit 2
}
[[ -f $PS5_SDL_SOURCE/.git/HEAD ]] || {
    echo "$PS5_SDL_SOURCE is not a git checkout; the builder archives the pinned commit" >&2
    exit 2
}

# A host archiver that cannot write the target's object format leaves a valid
# looking, empty archive behind (Apple's ar does exactly that), so the archive
# is checked for the code the title actually calls before it is used.
llvm_bin=$(dirname "${PS5_CLANG:?set PS5_CLANG to clang-18, llvm-ar and llvm-nm are looked up beside it}")
[[ -x $llvm_bin/llvm-ar && -x $llvm_bin/llvm-nm ]] || {
    echo "llvm-ar and llvm-nm are expected next to PS5_CLANG ($PS5_CLANG)" >&2
    exit 2
}

verify_prefix() {
    local listing symbols members
    listing=$(mktemp)
    symbols=$(mktemp)
    # the two listings are read from files: a `grep -q` on the end of a pipe
    # closes it early, and the writer's SIGPIPE would read as a failure
    "$llvm_bin/llvm-ar" t "$sdl2_prefix/lib/libSDL2.a" > "$listing"
    "$llvm_bin/llvm-nm" --defined-only "$sdl2_prefix/lib/libSDL2.a" > "$symbols"

    members=$(wc -l < "$listing" | tr -d ' ')
    for member in SDL_ps5audio.c.obj SDL_ps5g19.c.obj; do
        grep -qx "$member" "$listing" || {
            echo "libSDL2.a has no $member; the archive was not written for the console" >&2
            exit 1
        }
    done
    grep -q " SDL_OpenAudioDevice$" "$symbols" || {
        echo "libSDL2.a has no SDL_OpenAudioDevice; SDL_AUDIO is off" >&2
        exit 1
    }
    rm -f "$listing" "$symbols"
    echo "==> [sdl2] Prefix: $sdl2_prefix ($members archive members, audio included)"
}

# The SDK's own manifest is the identity the builder records into its receipt.
echo "==> [sdl2] Checking the ps5-opengl SDK manifest"
(cd "$PS5_OPENGL_PREFIX" && sha256sum --check --strict --quiet manifest.sha256)

if (( ${PS5_SDL2_FORCE:-0} )); then
    rm -rf "$native_build"
elif [[ -s $sdl2_prefix/lib/libSDL2.a ]]; then
    echo "==> [sdl2] $sdl2_prefix/lib/libSDL2.a is already built (PS5_SDL2_FORCE=1 to rebuild)"
    verify_prefix
    exit 0
fi

if git -C "$PS5_OPENGL_CHECKOUT" apply --check "$patch_file" 2>/dev/null; then
    echo "==> [sdl2] Applying $(basename "$patch_file")"
    git -C "$PS5_OPENGL_CHECKOUT" apply "$patch_file"
elif git -C "$PS5_OPENGL_CHECKOUT" apply --check --reverse "$patch_file" 2>/dev/null; then
    echo "==> [sdl2] $(basename "$patch_file") is already applied"
else
    echo "integration/SDL2/CMakeLists.txt is neither patched nor patchable;" >&2
    echo "check what changed in $PS5_OPENGL_CHECKOUT before building" >&2
    exit 2
fi

echo "==> [sdl2] Building SDL2 with the ps5-opengl builder"
python3 "$integration/build.py" native \
    --sdl-source "$PS5_SDL_SOURCE" \
    --sdk-prefix "$PS5_OPENGL_PREFIX" \
    --out "$native_build" \
    --payload-sdk "$PS5_PAYLOAD_SDK" \
    --compiler-wrapper "$PS5_BOILERPLATE/tooling/prospero-clang18"

verify_prefix
