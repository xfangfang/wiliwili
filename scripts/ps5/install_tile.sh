#!/usr/bin/env bash
# Build the payload that adds a wiliwili entry to the home screen of a PS5 and,
# when a host is given, upload it to the console and run it there.
#
# Usage:
#   scripts/ps5/install_tile.sh [-h HOST] [-d APP_DIR] [-b BUILD_DIR] [-P FTP_PORT] [-W WEB_PORT] [ARGS...]
#
# ARGS are passed to the installer payload, for example --uninstall, or the base
# URL of the websrv payload when it is not running on port 8080 of the console:
#   scripts/ps5/install_tile.sh -h ps5 -- http://127.0.0.1:8081
#
# The tile is a deeplink to <APP_DIR>/index.html, the page start the payload,
# both are served by the websrv payload. Run scripts/ps5/deploy.sh first so that
# the application itself is on the console.

set -e

PS5_HOST=""
APP_DIR="/data/homebrew/wiliwili"
BUILD_DIR="cmake-build-ps5"
FTP_PORT="2121"
WEB_PORT="8080"
INSTALL_ARGS=()

while getopts "h:d:b:P:W:" opt; do
    case ${opt} in
        h) PS5_HOST=$OPTARG ;;
        d) APP_DIR=$OPTARG ;;
        b) BUILD_DIR=$OPTARG ;;
        P) FTP_PORT=$OPTARG ;;
        W) WEB_PORT=$OPTARG ;;
    esac
done
shift $((OPTIND - 1))
INSTALL_ARGS=("$@")

# cd to wiliwili
cd "$(dirname "$0")/../.."

if [[ -z "${PS5_PAYLOAD_SDK}" ]]; then
    PS5_PAYLOAD_SDK="${HOME}/.local/opt/ps5-payload-sdk"
fi
if [[ ! -x "${PS5_PAYLOAD_SDK}/bin/prospero-clang" ]]; then
    echo "ps5-payload-sdk not found in ${PS5_PAYLOAD_SDK}, run scripts/ps5/install_sdk.sh" >&2
    exit 1
fi
if ! "${PS5_PAYLOAD_SDK}/bin/prospero-llvm-config" --bindir > /dev/null 2>&1; then
    echo "the SDK needs LLVM 18, install it and export LLVM_CONFIG, see scripts/ps5/README.md" >&2
    exit 1
fi

OUT_DIR="$(pwd)/${BUILD_DIR}/ps5"
mkdir -p "${OUT_DIR}"

# the assets are embedded into the payload, compile in the directory they live in
printf '%s\n' "[1/1] Building install_tile.elf"
( cd scripts/ps5/tile && "${PS5_PAYLOAD_SDK}/bin/prospero-clang" \
      -O2 -Wall -Werror -o "${OUT_DIR}/install_tile.elf" install_tile.c \
      -lkernel_sys -lSceAppInstUtil )

if [[ -z "${PS5_HOST}" ]]; then
    echo "Built ${OUT_DIR}/install_tile.elf, pass -h <ps5> to install it on a console" >&2
    exit 0
fi

upload() {
    curl --show-error --fail --silent --ftp-create-dirs \
        --upload-file "$1" "ftp://${PS5_HOST}:${FTP_PORT}$2"
}

echo "Uploading install_tile.elf -> ${APP_DIR}/install_tile.elf" >&2
upload "${OUT_DIR}/install_tile.elf" "${APP_DIR}/install_tile.elf"

echo "Running the installer on ${PS5_HOST}" >&2
curl --get \
     --show-error \
     --fail \
     --data-urlencode "path=${APP_DIR}/install_tile.elf" \
     --data-urlencode "cwd=${APP_DIR}" \
     --data-urlencode "args=${INSTALL_ARGS[*]}" \
     --data-urlencode "daemon=0" \
     --data-urlencode "pipe=1" \
     "http://${PS5_HOST}:${WEB_PORT}/hbldr"
