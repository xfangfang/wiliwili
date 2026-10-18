#!/usr/bin/env bash
# Upload a locally built wiliwili to a PS5 running the ps5-payload-dev websrv
# payload (ftpsrv on port 2121 + hbldr on port 8080) and start it.
#
# Usage:
#   scripts/ps5/deploy.sh [-h HOST] [-d APP_DIR] [-b BUILD_DIR] [-P FTP_PORT] [-W WEB_PORT] [-u] [ARGS...]
#
#   -u: skip uploading the resources directory
#
# The default layout matches what the homebrew.js launcher expects:
#   /data/homebrew/wiliwili/wiliwili.elf
#   /data/homebrew/wiliwili/resources/...

set -e

PS5_HOST="ps5"
APP_DIR="/data/homebrew/wiliwili"
BUILD_DIR="cmake-build-ps5"
FTP_PORT="2121"
WEB_PORT="8080"
UPLOAD_RESOURCES=1
RUN_ARGS=()

while getopts "h:d:b:P:W:u" opt; do
    case ${opt} in
        h) PS5_HOST=$OPTARG ;;
        d) APP_DIR=$OPTARG ;;
        b) BUILD_DIR=$OPTARG ;;
        P) FTP_PORT=$OPTARG ;;
        W) WEB_PORT=$OPTARG ;;
        u) UPLOAD_RESOURCES= ;;
    esac
done
shift $((OPTIND - 1))
RUN_ARGS=("$@")

# cd to wiliwili
cd "$(dirname "$0")/../.."

# the packaged executable is the one that can be started by its file name
LOCAL_ELF="${BUILD_DIR}/ps5/wiliwili.elf"
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

echo "Uploading ${LOCAL_ELF} -> ${APP_DIR}/wiliwili.elf" >&2
upload "${LOCAL_ELF}" "${APP_DIR}/wiliwili.elf"

if [[ -n "${UPLOAD_RESOURCES}" ]]; then
    echo "Uploading resources -> ${APP_DIR}/resources" >&2
    count=0
    while IFS= read -r -d '' file; do
        upload "${file}" "${APP_DIR}/resources/${file#resources/}"
        count=$((count + 1))
    done < <(find resources -type f -print0)
    echo "Uploaded ${count} resource files" >&2
fi

# the launcher entry and its icon, uploaded when the project has been packaged
for file in homebrew.js sce_sys/icon0.png; do
    if [[ -f "${BUILD_DIR}/ps5/${file}" ]]; then
        upload "${BUILD_DIR}/ps5/${file}" "${APP_DIR}/${file}"
    fi
done

echo "Launching ${APP_DIR}/wiliwili.elf on ${PS5_HOST}" >&2

curl --get \
     --show-error \
     --fail \
     --data-urlencode "path=${APP_DIR}/wiliwili.elf" \
     --data-urlencode "cwd=${APP_DIR}" \
     --data-urlencode "env=LD_LIBRARY_PATH=/user/homebrew/lib:/data/homebrew/lib:${APP_DIR}" \
     --data-urlencode "args=${RUN_ARGS[*]}" \
     --data-urlencode "daemon=0" \
     --data-urlencode "pipe=1" \
     "http://${PS5_HOST}:${WEB_PORT}/hbldr"
