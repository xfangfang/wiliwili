#!/usr/bin/env bash
# wiliwili - PS5 native title: fetch the third party sources the title links
# against. Everything is pinned to a tag or commit and verified, the download
# only happens once per dependency.
#
# The result is a directory of checkouts and prebuilt SDKs, used as
#
#   PS5_PAYLOAD_SDK=$prefix/ps5-payload-sdk
#   PS5_OPENGL_CHECKOUT=$prefix/ps5-opengl
#   PS5_OPENGL_PREFIX=$prefix/ps5-opengl-sdk-1.0.2/sdk
#   PS5_BOILERPLATE=$prefix/ps5-native-app-boilerplate
#   PS5_SDL_SOURCE=$prefix/SDL2
#
# which scripts/ps5/title/build_sdl2.sh and scripts/ps5/title/build_title.sh
# consume.
#
# Usage:
#   scripts/ps5/title/fetch_deps.sh [PREFIX]
#
# PREFIX defaults to $PS5_DEPS_PREFIX or ~/.cache/wiliwili-ps5/ps5-deps. It is
# a cache and not a temporary directory: macOS clears the temporary one, and
# the OpenGL SDK and the clean-room runtime that live there are not easy to
# get back.
#
# Environment:
#   PS5_DEPS_PREFIX    where the dependencies live (default ~/.cache/wiliwili-ps5/ps5-deps)
#   PS5_OPENGGL_VERSION  ps5-opengl release to use (default v1.0.2)

set -euo pipefail

prefix=${1:-${PS5_DEPS_PREFIX:-${HOME}/.cache/wiliwili-ps5/ps5-deps}}
opengl_version=${PS5_OPENGGL_VERSION:-v1.0.2}
opengl_commit=a519561626933257213d64a4a8bd5e02b55dceb2
opengl_sdk_sha256=30d8e51dd062739b629346102be5932728ba714bc50b1bdcdf08306c8843d9a4
# The converter must carry the RELRO anchor fix: earlier revisions built an
# image whose RELRO segment is incongruent with its file offset, and the loader
# refuses such an image before the application starts (CE-107750-0).
boilerplate_commit=44ae1dab529654eac795d2a7c4f5a0425102b8d7
sdl_commit=8c56053f13ca13a0c050de613706ff69eb615836

mkdir -p "${prefix}"

sha256_of() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | cut -d' ' -f1
    else
        shasum --algorithm 256 "$1" | cut -d' ' -f1
    fi
}

# fetch_repo URL REF DIR - clone URL at REF unless DIR already holds it. REF is
# either a tag or a commit hash, and a commit is fetched directly because the
# platforms this builds on do not all allow fetching an unreachable commit.
fetch_repo() {
    local url=$1 ref=$2 dir=$3
    if [[ -d ${dir}/.git ]] && [[ $(git -C "${dir}" rev-parse --verify --quiet "${ref}^{commit}" || true) ]]; then
        echo "==> [deps] $(basename "${dir}") already holds ${ref}"
        return 0
    fi

    echo "==> [deps] Cloning ${url} at ${ref}"
    rm -rf "${dir}"
    if git clone --quiet --depth 1 --branch "${ref}" "${url}" "${dir}" 2>/dev/null; then
        return 0
    fi

    rm -rf "${dir}"
    mkdir -p "${dir}"
    git -C "${dir}" init --quiet
    git -C "${dir}" remote add origin "${url}"
    git -C "${dir}" fetch --quiet --depth 1 origin "${ref}"
    git -C "${dir}" checkout --quiet FETCH_HEAD
}

# fetch_file URL SHA256 DEST - download and verify, unless DEST is already the
# verified file.
fetch_file() {
    local url=$1 sha256=$2 dest=$3
    if [[ -f ${dest} ]] && [[ $(sha256_of "${dest}") == "${sha256}" ]]; then
        echo "==> [deps] $(basename "${dest}") is already verified"
        return 0
    fi

    echo "==> [deps] Downloading ${url}"
    mkdir -p "$(dirname "${dest}")"
    local temporary="${dest}.part"
    curl --location --fail --retry 3 --retry-delay 2 --output "${temporary}" "${url}"
    local actual
    actual=$(sha256_of "${temporary}")
    if [[ ${actual} != "${sha256}" ]]; then
        rm -f "${temporary}"
        echo "checksum mismatch for ${url}" >&2
        echo "  expected ${sha256}" >&2
        echo "  actual   ${actual}" >&2
        exit 1
    fi
    mv "${temporary}" "${dest}"
}

# the payload development environment, the same one the payload build uses
if [[ -x ${prefix}/ps5-payload-sdk/bin/prospero-clang ]]; then
    echo "==> [deps] ps5-payload-sdk is already installed in ${prefix}/ps5-payload-sdk"
else
    "$(dirname "$0")/../install_sdk.sh" "${prefix}/ps5-payload-sdk"
fi

# the OpenGL implementation for the console, its SDK release carries the static
# libraries, the AGC stubs the title imports from and the SDL2 integration
# The upstream repositories are not reachable any more, so the build uses the
# mirrors: the hashes below are what the build is verified against, and they
# are checked on every fetch.
fetch_repo https://github.com/carlchina/ps5-opengl.git "${opengl_version}" "${prefix}/ps5-opengl"
if [[ $(git -C "${prefix}/ps5-opengl" rev-parse HEAD) != "${opengl_commit}" ]]; then
    echo "note: ${opengl_version} is not the ${opengl_commit:0:12} the build was verified against" >&2
fi

sdk_archive="${prefix}/ps5-opengl-sdk-${opengl_version#v}.tar.gz"
sdk_root="${prefix}/ps5-opengl-sdk-${opengl_version#v}"
fetch_file \
    "https://github.com/carlchina/ps5-opengl/releases/download/sdk-v${opengl_version#v}/ps5-opengl-sdk-${opengl_version#v}.tar.gz" \
    "${opengl_sdk_sha256}" "${sdk_archive}"
if [[ ! -f ${sdk_root}/sdk/manifest.sha256 ]]; then
    echo "==> [deps] Extracting $(basename "${sdk_archive}")"
    rm -rf "${sdk_root}"
    tar --extract --gzip --directory="${prefix}" --file="${sdk_archive}"
fi
[[ -f ${sdk_root}/sdk/manifest.sha256 ]] || {
    echo "${sdk_root}/sdk/manifest.sha256 is missing, the archive is not the SDK release" >&2
    exit 1
}

# the native application toolchain: the module converter, the startup objects
# and the runtime module that a folder title is loaded with
fetch_repo https://github.com/carlchina/ps5-native-app-boilerplate.git \
    "${boilerplate_commit}" "${prefix}/ps5-native-app-boilerplate"
# a payload has no dynamic loader to run it otherwise
chmod +x "${prefix}/ps5-native-app-boilerplate/tooling/prospero-clang18"
"${prefix}/ps5-native-app-boilerplate/tools/validate-assets.sh" \
    "${prefix}/ps5-native-app-boilerplate/sce_sys" >/dev/null

# SDL2 stays a git checkout: the ps5-opengl builder archives the pinned commit
# with git to verify it, which a plain source tarball cannot satisfy
# The commit is in the console's fork of SDL, not in the upstream repository.
fetch_repo https://github.com/carlchina/SDL.git "${sdl_commit}" "${prefix}/SDL2"

cat >&2 <<EOF

==> [deps] Dependencies are in ${prefix}:
    PS5_PAYLOAD_SDK=${prefix}/ps5-payload-sdk
    PS5_OPENGL_CHECKOUT=${prefix}/ps5-opengl
    PS5_OPENGL_PREFIX=${sdk_root}/sdk
    PS5_BOILERPLATE=${prefix}/ps5-native-app-boilerplate
    PS5_SDL_SOURCE=${prefix}/SDL2

Next: scripts/ps5/title/build_native_tool.sh and scripts/ps5/title/build_sdl2.sh
EOF
