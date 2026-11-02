#!/usr/bin/env bash
# Upload a locally built wiliwili to a PS5 running the ps5-payload-dev websrv
# payload (ftpsrv on port 2121 + hbldr on port 8080) and start it.
#
# Usage:
#   scripts/ps5/deploy.sh [-h HOST] [-d APP_DIR] [-b BUILD_DIR] [-P FTP_PORT] [-W WEB_PORT] [-u] [ARGS...]
#
#   -u: skip uploading the resources directory
#   -L: upload the mesa libraries even when the console already has them
#
# The default layout is the one the homebrew launcher expects to find, the app
# shows up in the launcher page served by websrv (http://<ps5>:8080/):
#   /data/homebrew/wiliwili/eboot.elf
#   /data/homebrew/wiliwili/lib/libGL.so.1
#   /data/homebrew/wiliwili/resources/...
#
# Extra arguments are passed to the application (use -- in front of them when
# they start with a dash, for example: scripts/ps5/deploy.sh -h ps5 -- -d)

set -e

PS5_HOST="ps5"
APP_DIR="/data/homebrew/wiliwili"
BUILD_DIR="cmake-build-ps5"
FTP_PORT="2121"
WEB_PORT="8080"
UPLOAD_RESOURCES=1
FORCE_LIBS=""
RUN_ARGS=()

while getopts "h:d:b:P:W:uL" opt; do
    case ${opt} in
        h) PS5_HOST=$OPTARG ;;
        d) APP_DIR=$OPTARG ;;
        b) BUILD_DIR=$OPTARG ;;
        P) FTP_PORT=$OPTARG ;;
        W) WEB_PORT=$OPTARG ;;
        u) UPLOAD_RESOURCES= ;;
        L) FORCE_LIBS=1 ;;
    esac
done
shift $((OPTIND - 1))
RUN_ARGS=("$@")

# cd to wiliwili
cd "$(dirname "$0")/../.."

# the packaged executable, eboot.elf is the name the homebrew launcher starts
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

upload() {
    local local_path="$1"
    local remote_path="$2"
    curl --show-error --fail --silent --ftp-create-dirs \
        --upload-file "${local_path}" \
        "ftp://${PS5_HOST}:${FTP_PORT}${remote_path}"
}

# the executable links against libGL.so.1 of the pacbrew mesa, which pulls in
# libgallium-26.2.0.so, which pulls in libvulkan.so.1. The build output carries
# the three files (cmake-build-ps5/ps5/lib), the launcher and this script put the
# application's lib directory on LD_LIBRARY_PATH, so they only have to be
# uploaded once
LIBS_DIR="${BUILD_DIR}/ps5/lib"

# names of the files in a remote directory, empty when the server cannot list it
remote_list() {
    curl --show-error --silent --list-only "ftp://${PS5_HOST}:${FTP_PORT}$1" 2>/dev/null || true
}

upload_libs() {
    if [[ ! -d "${LIBS_DIR}" ]]; then
        echo "No ${LIBS_DIR}, the console needs libGL.so.1, libgallium and libvulkan.so.1 itself" >&2
        return
    fi

    local existing=""
    if [[ -z "${FORCE_LIBS}" ]]; then
        existing=$(remote_list "${APP_DIR}/lib/")
    fi

    local count=0 file name
    for file in "${LIBS_DIR}"/*; do
        [[ -f "${file}" ]] || continue
        name=$(basename "${file}")
        if [[ -n "${existing}" ]] && grep -qxF "${name}" <<< "${existing}"; then
            continue
        fi
        echo "Uploading ${name} -> ${APP_DIR}/lib/${name}" >&2
        upload "${file}" "${APP_DIR}/lib/${name}"
        count=$((count + 1))
    done
    echo "Uploaded ${count} libraries" >&2
}

echo "Uploading ${LOCAL_ELF} -> ${APP_DIR}/eboot.elf" >&2
upload "${LOCAL_ELF}" "${APP_DIR}/eboot.elf"

if [[ -n "${UPLOAD_RESOURCES}" ]]; then
    echo "Uploading resources -> ${APP_DIR}/resources" >&2
    count=0
    while IFS= read -r -d '' file; do
        upload "${file}" "${APP_DIR}/resources/${file#resources/}"
        count=$((count + 1))
    done < <(find resources -type f -print0)
    echo "Uploaded ${count} resource files" >&2
fi

# the launcher entry, its icon and the web app page used by the optional home
# screen tile, uploaded when the project has been packaged
for file in homebrew.js index.html sce_sys/icon0.png; do
    if [[ -f "${BUILD_DIR}/ps5/${file}" ]]; then
        upload "${BUILD_DIR}/ps5/${file}" "${APP_DIR}/${file}"
    fi
done

# the optional tile installer, started separately with scripts/ps5/install_tile.sh
if [[ -f "${BUILD_DIR}/ps5/install_tile.elf" ]]; then
    upload "${BUILD_DIR}/ps5/install_tile.elf" "${APP_DIR}/install_tile.elf"
fi

# the libraries the executable needs, skipped when the console already has them
upload_libs

echo "Launching ${APP_DIR}/eboot.elf on ${PS5_HOST}" >&2

curl --get \
     --show-error \
     --fail \
     --data-urlencode "path=${APP_DIR}/eboot.elf" \
     --data-urlencode "cwd=${APP_DIR}" \
     --data-urlencode "env=LD_LIBRARY_PATH=${APP_DIR}/lib:/data/homebrew/lib:/user/homebrew/lib" \
     --data-urlencode "args=${RUN_ARGS[*]}" \
     --data-urlencode "daemon=0" \
     --data-urlencode "pipe=1" \
     "http://${PS5_HOST}:${WEB_PORT}/hbldr"
