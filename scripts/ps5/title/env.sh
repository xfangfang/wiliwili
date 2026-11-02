#!/usr/bin/env bash
# wiliwili - PS5 native title: the environment the title is built in.
#
# Source this before building:
#
#   source scripts/ps5/title/env.sh
#   scripts/ps5/title/build_title.sh
#
# The dependencies are fetched by scripts/ps5/title/fetch_deps.sh, which puts
# them in the directory below. They live in the cache and not in a temporary
# directory: macOS clears the temporary one, and the OpenGL SDK and the
# clean-room runtime that live there are not easy to get back.
#
# Every path can be overridden from the environment.

deps=${PS5_DEPS_DIR:-${HOME}/.cache/wiliwili-ps5/ps5-deps}

export PS5_PAYLOAD_SDK=${PS5_PAYLOAD_SDK:-${deps}/ps5-payload-sdk}
export PS5_OPENGL_PREFIX=${PS5_OPENGL_PREFIX:-${deps}/ps5-opengl-sdk-1.0.2/sdk}
export PS5_OPENGL_CHECKOUT=${PS5_OPENGL_CHECKOUT:-${deps}/ps5-opengl}
export PS5_BOILERPLATE=${PS5_BOILERPLATE:-${deps}/ps5-native-app-boilerplate}
export PS5_SDL_SOURCE=${PS5_SDL_SOURCE:-${deps}/SDL2}
export PS5_SDL2_PREFIX=${PS5_SDL2_PREFIX:-${deps}/ps5opengl/build/sdl2-native/sdk}
export PS5_NATIVE_TOOL=${PS5_NATIVE_TOOL:-${deps}/ps5-native-app-boilerplate/build-host/ps5-native-tool}

# The console's toolchain, and the LLVM it needs. The layout is the one the
# platform's llvm-18 package uses: Homebrew on macOS, the distribution's
# /usr/lib/llvm-18 elsewhere (as on the CI runner).
if [[ $(uname -s) == Darwin ]]; then
    export PS5_CLANG=${PS5_CLANG:-/opt/homebrew/opt/llvm@18/bin/clang}
    export LLVM_CONFIG=${LLVM_CONFIG:-/opt/homebrew/opt/llvm@18/bin/llvm-config}
    export CXX=${CXX:-/opt/homebrew/opt/llvm@18/bin/clang++}
    # The boilerplate's scripts call sha256sum with the GNU options, which macOS
    # does not have; the checkout carries a stand-in for it.
    export PATH="${PS5_BOILERPLATE}/tools/macos:${PATH}"
else
    export PS5_CLANG=${PS5_CLANG:-/usr/lib/llvm-18/bin/clang}
    export LLVM_CONFIG=${LLVM_CONFIG:-/usr/lib/llvm-18/bin/llvm-config}
    export CXX=${CXX:-/usr/lib/llvm-18/bin/clang++}
fi

echo "==> [title] Dependencies: ${deps}"
