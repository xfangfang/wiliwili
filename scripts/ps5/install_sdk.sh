#!/usr/bin/env bash
# Download the prebuilt ps5-payload development environment (a cross compilation
# SDK with SDL2, ffmpeg, mesa, libass, curl and friends) published by the
# ps5-payload-dev project.
#
# Usage:
#   scripts/ps5/install_sdk.sh [TARGET_DIR]
#
# The default target directory is ~/.local/opt/ps5-payload-sdk, afterwards
# export PS5_PAYLOAD_SDK to that directory (scripts/build_ps5.sh does that
# automatically when PS5_PAYLOAD_SDK is not set).

set -e

TARGET_DIR="${1:-${HOME}/.local/opt/ps5-payload-sdk}"
# version of the pacbrew dev image, see https://github.com/ps5-payload-dev/pacbrew-repo/releases
RELEASE="v0.41.1"
SHA256="e811bc2e4676c5bb7f80a8fe928d2985dce599996a5ca6b54da3e72699287887"
URL="${PS5_DEV_URL:-https://github.com/ps5-payload-dev/pacbrew-repo/releases/download/${RELEASE}/ps5-payload-dev.tar.gz}"

if [[ -x "${TARGET_DIR}/bin/prospero-clang" ]]; then
    echo "ps5-payload-sdk is already installed in ${TARGET_DIR}" >&2
    exit 0
fi

ARCHIVE="$(mktemp -t ps5-payload-dev-XXXXXX.tar.gz)"
trap 'rm -f "${ARCHIVE}"' EXIT

echo "Downloading ${URL}" >&2
curl --location --fail --retry 3 --retry-delay 2 --output "${ARCHIVE}" "${URL}"

if [[ "${PS5_DEV_URL}" == "" ]]; then
    # sha256sum on linux, shasum on macOS (both print "hash  file"), the
    # --check options of the two are not interchangeable
    SUM="$(sha256sum "${ARCHIVE}" 2>/dev/null | cut -d' ' -f1)"
    [[ -n "${SUM}" ]] || SUM="$(shasum --algorithm 256 "${ARCHIVE}" | cut -d' ' -f1)"
    if [[ "${SUM}" != "${SHA256}" ]]; then
        echo "Checksum mismatch, the download is corrupted or the release has been updated" >&2
        exit 1
    fi
fi

echo "Extracting to ${TARGET_DIR}" >&2
mkdir -p "${TARGET_DIR}"
tar --extract --gzip --strip-components=2 --directory="${TARGET_DIR}" --file="${ARCHIVE}"

if [[ ! -x "${TARGET_DIR}/bin/prospero-clang" ]]; then
    echo "Failed to install ps5-payload-sdk into ${TARGET_DIR}" >&2
    exit 1
fi

echo "ps5-payload-sdk has been installed into ${TARGET_DIR}" >&2
echo "export PS5_PAYLOAD_SDK=${TARGET_DIR}" >&2
