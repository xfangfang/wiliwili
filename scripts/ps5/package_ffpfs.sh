#!/usr/bin/env bash
# Package a PS5 native title folder into a compressed PFS container (.ffpfsc)
# using ps5-dump-forge (https://github.com/quer3q/ps5-dump-forge).
#
# The resulting .ffpfsc image can be mounted and launched directly on a jailbroken PS5
# by loaders such as ShadowMountPlus without unpacking hundreds of files, saving
# over 55% storage space with zlib level 9 compression.
#
# Usage:
#   scripts/ps5/package_ffpfs.sh [-i INPUT_DIR] [-o OUT_DIR] [-n OUT_NAME] [-l LEVEL] [-t TOOL] [-h]
#
# Options:
#   -i INPUT_DIR   directory of the built native title (default: dist/PPSA99099)
#   -o OUT_DIR     directory where .ffpfsc will be written (default: dist)
#   -n OUT_NAME    base name of the output file without extension
#   -l LEVEL       compression level for ffpfsc (0..9, default: 9)
#   -t TOOL        path to ps5-dump-forge executable (or set PS5_DUMP_FORGE)
#   -h             show this help

set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
cd "$root"

INPUT_DIR=""
OUT_DIR=""
OUT_NAME=""
COMPRESS_LEVEL="9"
FORGE_BIN="${PS5_DUMP_FORGE:-}"
DEFAULT_FORGE_VERSION="0.5.0"

usage() {
    sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

while getopts "i:o:n:l:t:h" opt; do
    case ${opt} in
        i) INPUT_DIR=$OPTARG ;;
        o) OUT_DIR=$OPTARG ;;
        n) OUT_NAME=$OPTARG ;;
        l) COMPRESS_LEVEL=$OPTARG ;;
        t) FORGE_BIN=$OPTARG ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

# Auto-locate INPUT_DIR if not provided
if [[ -z "${INPUT_DIR}" ]]; then
    if [[ -n "${PS5_TITLE_OUT_DIR:-}" && -d "${PS5_TITLE_OUT_DIR}" ]]; then
        INPUT_DIR="${PS5_TITLE_OUT_DIR}"
    elif [[ -d "dist/PPSA99099" ]]; then
        INPUT_DIR="dist/PPSA99099"
    else
        # Find any title folder in dist/
        for candidate in dist/*; do
            if [[ -d "${candidate}" && -f "${candidate}/eboot.bin" ]]; then
                INPUT_DIR="${candidate}"
                break
            fi
        done
    fi
fi

if [[ -z "${INPUT_DIR}" || ! -d "${INPUT_DIR}" ]]; then
    echo "Error: Title folder not found. Specify with -i <path> or build it first with:" >&2
    echo "  cmake --build cmake-build-ps5-title --target wiliwili.title" >&2
    exit 1
fi

if [[ ! -f "${INPUT_DIR}/eboot.bin" ]]; then
    echo "Error: '${INPUT_DIR}' does not appear to be a valid PS5 title folder (missing eboot.bin)." >&2
    exit 1
fi

TITLE_ID=$(basename "${INPUT_DIR}")
if [[ -f "${INPUT_DIR}/sce_sys/param.json" ]]; then
    TITLE_ID_JSON=$(sed -n 's/.*"titleId"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "${INPUT_DIR}/sce_sys/param.json" | head -1)
    if [[ -n "${TITLE_ID_JSON}" ]]; then
        TITLE_ID="${TITLE_ID_JSON}"
    fi
fi

# Detect version tag if available
BUILD_VERSION=""
if [[ -f "CMakeLists.txt" ]]; then
    v_maj=$(sed -n 's/.*set(VERSION_MAJOR "\([0-9]*\)").*/\1/p' CMakeLists.txt)
    v_min=$(sed -n 's/.*set(VERSION_MINOR "\([0-9]*\)").*/\1/p' CMakeLists.txt)
    v_rev=$(sed -n 's/.*set(VERSION_REVISION "\([0-9]*\)").*/\1/p' CMakeLists.txt)
    v_bld=$(sed -n 's/.*set(VERSION_BUILD "\([0-9]*\)").*/\1/p' CMakeLists.txt)
    if [[ -n "${v_maj}" && -n "${v_min}" && -n "${v_rev}" ]]; then
        if [[ -n "${v_bld}" && "${v_bld}" != "0" ]]; then
            BUILD_VERSION="v${v_maj}.${v_min}.${v_rev}.${v_bld}"
        else
            BUILD_VERSION="v${v_maj}.${v_min}.${v_rev}"
        fi
    fi
fi

if [[ -z "${BUILD_VERSION}" && -f "${INPUT_DIR}/sce_sys/param.json" ]]; then
    CONTENT_VER=$(sed -n 's/.*"contentVersion"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "${INPUT_DIR}/sce_sys/param.json" | head -1)
    if [[ -n "${CONTENT_VER}" ]]; then
        BUILD_VERSION="v${CONTENT_VER#0}"
    fi
fi

# Set default OUT_DIR
if [[ -z "${OUT_DIR}" ]]; then
    OUT_DIR=$(dirname "${INPUT_DIR}")
fi
mkdir -p "${OUT_DIR}"

# Set default OUT_NAME
if [[ -z "${OUT_NAME}" ]]; then
    if [[ -n "${BUILD_VERSION}" ]]; then
        OUT_NAME="wiliwili-PS5-${BUILD_VERSION}"
    else
        OUT_NAME="${TITLE_ID}"
    fi
fi

# Locate or download ps5-dump-forge
find_or_download_forge() {
    if [[ -n "${FORGE_BIN}" && -x "${FORGE_BIN}" ]]; then
        echo "${FORGE_BIN}"
        return 0
    fi

    if command -v ps5-dump-forge >/dev/null 2>&1; then
        command -v ps5-dump-forge
        return 0
    fi

    local cache_dir="${XDG_CACHE_HOME:-$HOME/.cache}/ps5-dump-forge"
    local cached_bin="${cache_dir}/ps5-dump-forge"
    if [[ -x "${cached_bin}" ]]; then
        echo "${cached_bin}"
        return 0
    fi

    echo "==> [ffpfsc] ps5-dump-forge not found; downloading from GitHub releases..." >&2
    mkdir -p "${cache_dir}"

    local os arch tag version
    os=$(uname -s)
    arch=$(uname -m)

    # Try resolving the latest release tag
    tag=$(curl -sI https://github.com/quer3q/ps5-dump-forge/releases/latest | grep -i "^location:" | sed -E 's/.*tag\/(v?[0-9.]+).*/\1/' | tr -d '\r\n' || true)
    if [[ -z "${tag}" ]]; then
        tag="v${DEFAULT_FORGE_VERSION}"
    fi
    version="${tag#v}"

    local download_url=""
    local archive_name=""

    if [[ "${os}" == "Darwin" ]]; then
        archive_name="ps5-dump-forge-${version}-macos-universal.zip"
        download_url="https://github.com/quer3q/ps5-dump-forge/releases/download/${tag}/${archive_name}"
        echo "==> [ffpfsc] Downloading ${download_url} ..." >&2
        curl -fSL "${download_url}" -o "${cache_dir}/${archive_name}"
        unzip -q -o -j "${cache_dir}/${archive_name}" "*/ps5-dump-forge" -d "${cache_dir}"
        rm -f "${cache_dir}/${archive_name}"
    elif [[ "${os}" == "Linux" ]]; then
        if [[ "${arch}" == "aarch64" || "${arch}" == "arm64" ]]; then
            archive_name="ps5-dump-forge-${version}-linux-arm64.tar.gz"
        else
            archive_name="ps5-dump-forge-${version}-linux-x86-64.tar.gz"
        fi
        download_url="https://github.com/quer3q/ps5-dump-forge/releases/download/${tag}/${archive_name}"
        echo "==> [ffpfsc] Downloading ${download_url} ..." >&2
        curl -fSL "${download_url}" -o "${cache_dir}/${archive_name}"
        tar -xzf "${cache_dir}/${archive_name}" -C "${cache_dir}" --wildcards '*/ps5-dump-forge' --strip-components=1
        rm -f "${cache_dir}/${archive_name}"
    else
        echo "Error: Unsupported OS for auto-download: ${os}. Please set PS5_DUMP_FORGE manually." >&2
        exit 1
    fi

    chmod +x "${cached_bin}"
    echo "${cached_bin}"
}

FORGE_BIN=$(find_or_download_forge)
if [[ ! -x "${FORGE_BIN}" ]]; then
    echo "Error: Failed to obtain ps5-dump-forge executable." >&2
    exit 1
fi

TARGET_FILE="${OUT_DIR}/${OUT_NAME}.ffpfsc"
echo "==> [ffpfsc] Packing '${INPUT_DIR}' into '${TARGET_FILE}' (zlib level ${COMPRESS_LEVEL}) ..."

"${FORGE_BIN}" convert "${INPUT_DIR}" --to ffpfsc --inner ffpfs --level "${COMPRESS_LEVEL}" -o "${TARGET_FILE}"

# Also create Title ID named copy or symlink if output was named differently
if [[ "${OUT_NAME}" != "${TITLE_ID}" ]]; then
    TITLE_TARGET="${OUT_DIR}/${TITLE_ID}.ffpfsc"
    cp -f "${TARGET_FILE}" "${TITLE_TARGET}"
    echo "==> [ffpfsc] Created copy: '${TITLE_TARGET}'"
fi

echo "==> [ffpfsc] Inspecting generated image:"
"${FORGE_BIN}" inspect "${TARGET_FILE}"

echo ""
echo "====================================================================="
echo "PS5 compressed PFS image (.ffpfsc) built successfully:"
echo "  ${TARGET_FILE}"
if [[ "${OUT_NAME}" != "${TITLE_ID}" ]]; then
    echo "  ${OUT_DIR}/${TITLE_ID}.ffpfsc"
fi
echo ""
echo "Installation on PS5 with ShadowMountPlus:"
echo "  1. Copy ${OUT_NAME}.ffpfsc (or ${TITLE_ID}.ffpfsc) to a USB drive or"
echo "     to /data/homebrew/ on your PS5."
echo "  2. ShadowMountPlus will automatically detect and mount it."
echo "====================================================================="
