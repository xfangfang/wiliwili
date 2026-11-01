#!/usr/bin/env bash
# Stage the PS5 build as an application folder and pack it into a debug fPKG, the
# package format a jailbroken PS5 installs with the system package installer.
#
# This is an experiment: the package builds, but it has never been started on a
# console, see the "Status" section of scripts/ps5/README.md for the parts that
# still have to be proven (the shared libraries of the application, the resource
# directory, access to /data/...).
#
# Usage:
#   scripts/ps5/package_fpkg.sh [-b BUILD_DIR] [-o OUT_DIR] [-t TITLE_ID]
#                              [-c CONTENT_ID] [-n TITLE] [-v VERSION] [-j JOBS] [-s]
#
#   -b: directory of the packaged build output (default cmake-build-ps5/ps5)
#   -o: directory the staged folder and the package are written to (default cmake-build-ps5/fpkg)
#   -t: PS5 title id, nine characters (default PPSA99099)
#   -c: content id, 36 characters (default UP9000-<title id>_00-<title>, padded)
#   -n: title shown on the home screen (default wiliwili)
#   -v: content version (default 01.000.000)
#   -j: packer worker count, 0 = one per core (default 0)
#   -s: stage the application folder only, do not pack it
#
# The packer is taken from $PS5_FPKG_TOOL, or from $PATH when the variable is
# empty. Only the "ffpfsc-pkg-tool" of https://github.com/knutwurst/ps5-ultrapack
# (a command line front end of https://github.com/drakmor/LibProsperoPkg, GPL-3.0)
# is known to produce packages that install and launch: it generates the
# icon0.dds the console requires and fake-signs the eboot. See
# scripts/ps5/README.md for how to obtain it.

set -euo pipefail

BUILD_DIR="cmake-build-ps5/ps5"
OUT_DIR="cmake-build-ps5/fpkg"
TITLE_ID="PPSA99099"
CONTENT_ID=""
TITLE="wiliwili"
VERSION="01.600.005"
JOBS="0"
STAGE_ONLY=""

usage() {
    sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

while getopts "b:o:t:c:n:v:j:sh" opt; do
    case ${opt} in
        b) BUILD_DIR=$OPTARG ;;
        o) OUT_DIR=$OPTARG ;;
        t) TITLE_ID=$OPTARG ;;
        c) CONTENT_ID=$OPTARG ;;
        n) TITLE=$OPTARG ;;
        v) VERSION=$OPTARG ;;
        j) JOBS=$OPTARG ;;
        s) STAGE_ONLY=1 ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

# cd to wiliwili
cd "$(dirname "$0")/../.."

# the packaged executable, the build deploys it as eboot.elf (scripts/ps5/deploy.sh)
LOCAL_ELF="${BUILD_DIR}/ps5/eboot.elf"
if [[ ! -f "${LOCAL_ELF}" ]]; then
    LOCAL_ELF="${BUILD_DIR}/ps5/wiliwili.elf"
fi
if [[ ! -f "${LOCAL_ELF}" ]]; then
    LOCAL_ELF="${BUILD_DIR}/wiliwili"
fi
if [[ ! -f "${LOCAL_ELF}" ]]; then
    echo "ELF not found: ${LOCAL_ELF}, build it first" >&2
    exit 1
fi

ICON="scripts/ps5/sce_sys/icon0.png"
if [[ ! -f "${ICON}" ]]; then
    echo "Icon not found: ${ICON}" >&2
    exit 1
fi

if [[ ${#TITLE_ID} -ne 9 ]]; then
    echo "Title id has to be nine characters: ${TITLE_ID}" >&2
    exit 1
fi

if [[ -z "${CONTENT_ID}" ]]; then
    # UP9000 is the publisher id the free tools write, the last field is a sixteen
    # character label that has to be unique per package
    LABEL=$(echo "${TITLE}" | tr -cd '[:alnum:]' | tr '[:lower:]' '[:upper:]' | cut -c 1-16)
    LABEL=$(printf '%-16s' "${LABEL}" | tr ' ' '0')
    CONTENT_ID="UP9000-${TITLE_ID}_00-${LABEL}"
fi

if [[ ${#CONTENT_ID} -ne 36 ]]; then
    echo "Content id has to be 36 characters: ${CONTENT_ID}" >&2
    exit 1
fi

STAGE_DIR="${OUT_DIR}/wiliwili"

echo "Staging ${STAGE_DIR}" >&2
rm -rf "${STAGE_DIR}"
mkdir -p "${STAGE_DIR}/sce_sys"

cp "${LOCAL_ELF}" "${STAGE_DIR}/eboot.bin"
chmod 755 "${STAGE_DIR}/eboot.bin"

# same metadata layout as LibProsperoPkg's HomebrewTest sample
# param.json carries the master version as two two-digit fields
MASTER_VERSION=$(echo "${VERSION}" | awk -F. '{printf "%s.%s", substr($1, 1, 2), substr($2, 1, 2)}')
sed -e "s|@CONTENT_ID@|${CONTENT_ID}|g" \
    -e "s|@TITLE_ID@|${TITLE_ID}|g" \
    -e "s|@CONCEPT_ID@|${TITLE_ID: -5}|g" \
    -e "s|@TITLE@|${TITLE}|g" \
    -e "s|@VERSION@|${VERSION}|g" \
    -e "s|@MASTER_VERSION@|${MASTER_VERSION}|g" \
    scripts/ps5/fpkg/param.json > "${STAGE_DIR}/sce_sys/param.json"

cp "${ICON}" "${STAGE_DIR}/sce_sys/icon0.png"

# The executable links against libGL.so.1 of the pacbrew mesa, which pulls in
# libgallium and libvulkan. A package is started without LD_LIBRARY_PATH, so the
# loader has to find them in a directory it searches, the package root is one of
# the candidates it may look in. The payload deployment keeps the same files in
# its lib directory, see scripts/ps5/README.md.
if [[ -d "${BUILD_DIR}/lib" ]]; then
    cp "${BUILD_DIR}/lib/"* "${STAGE_DIR}/"
fi

if [[ -n "${STAGE_ONLY}" ]]; then
    echo "Staged ${STAGE_DIR}, not packing (-s)" >&2
    exit 0
fi

PACKER="${PS5_FPKG_TOOL:-}"
if [[ -z "${PACKER}" ]]; then
    PACKER=$(command -v ffpfsc-pkg-tool || true)
fi

if [[ ! -x "${PACKER}" ]]; then
    cat >&2 <<EOF
Packer not found, staged the application folder only: ${STAGE_DIR}

Set PS5_FPKG_TOOL to the command line packer of LibProsperoPkg
(https://github.com/drakmor/LibProsperoPkg, GPL-3.0), the build of
https://github.com/knutwurst/ps5-ultrapack is the one known to work:

  # macOS (arm64), the tool ships inside the application of a release
  curl -L -o ps5-ultrapack.zip \\
      https://github.com/knutwurst/ps5-ultrapack/releases/latest/download/PS5-UltraPack-macos-arm64.zip
  unzip ps5-ultrapack.zip && export PS5_FPKG_TOOL=\$PWD/ffpfsc-pkg-tool

  # or build it from source, needs the .NET SDK 9 or newer
  git clone https://github.com/knutwurst/ps5-ultrapack
  cd ps5-ultrapack && ./BUILD_PKG_TOOL.sh
  export PS5_FPKG_TOOL=\$PWD/backend/native/ffpfsc-pkg-tool

Then pack the staged folder:

  "\$PS5_FPKG_TOOL" build "${STAGE_DIR}" "${OUT_DIR}" \\
      --content-id "${CONTENT_ID}" --title-id "${TITLE_ID}" --title "${TITLE}" -j ${JOBS}
EOF
    exit 1
fi

echo "Packing with ${PACKER}" >&2
"${PACKER}" build "${STAGE_DIR}" "${OUT_DIR}" \
    --content-id "${CONTENT_ID}" \
    --title-id "${TITLE_ID}" \
    --title "${TITLE}" \
    --version "${VERSION}" \
    -j "${JOBS}"

PKG=$(ls -1 "${OUT_DIR}"/*.pkg 2>/dev/null | head -n 1 || true)
if [[ -z "${PKG}" ]]; then
    echo "No package was produced in ${OUT_DIR}" >&2
    exit 1
fi
echo "Built ${PKG}" >&2

cat >&2 <<EOF

Install it from Settings -> Debug Settings -> Game -> Package Installer with the
file in the root of a USB drive.

Requirements and known gaps of this route, it has never been started from a
package on a console:
  - kstuff with fself and fPKG support has to be active while the application is
    started (kstuff-lite 1.11 or newer, the packer is verified with 1.13). On a
    console without it every title fails with CE-100008-9 before any of its code
    runs, which is what the first launch on a test console reported
  - wiliwili reads its resources from /data/homebrew/wiliwili/resources/, a package
    does not change that path: scripts/ps5/deploy.sh uploads them (and needs websrv
    running), or use any FTP client, ftpsrv listens on port 2121
  - eboot.bin links against libGL.so.1 and what it pulls in, and a package is
    started by the system without the LD_LIBRARY_PATH that scripts/ps5/homebrew.js
    sets, so the loader has to find them in a directory it searches. The three
    files are packed next to eboot.bin; whether the loader looks into the
    application directory is not known, the pacbrew shared libraries carry a bare
    soname and the SDK's hello_so sample gives its libraries absolute ones for
    exactly this reason
  - a package runs without websrv, so nothing patches the process the way the
    payload loader does
EOF
