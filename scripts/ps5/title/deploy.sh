#!/usr/bin/env bash
# wiliwili - PS5 native title: publish the title folder to a console that runs
# an FTP service.
#
# The console reads a folder title from /data/homebrew/<TITLE_ID>/, so the whole
# folder is uploaded file by file. Every file is first written under a hidden
# name beside its destination and renamed once it arrived, and eboot.bin and
# sce_sys/param.json are published last, so a half finished upload never leaves
# a folder behind that the loader would try to start.
#
# Layout created on the console:
#   /data/homebrew/<TITLE_ID>/eboot.bin
#   /data/homebrew/<TITLE_ID>/sce_module/libc.prx
#   /data/homebrew/<TITLE_ID>/sce_sys/{param.json,icon0.png}
#   /data/homebrew/<TITLE_ID>/resources/...
#
# The console has to be told about the new folder afterwards: reload the
# ShadowMountPlus payload (or restart the console) and the title appears on the
# home screen.
#
# Usage:
#   scripts/ps5/title/deploy.sh [-h HOST] [-p PORT] [-d DIST_DIR] [-t TITLE_ID] [--undeploy]
#
# Options:
#   -h HOST       console address (default: $PS5_HOST or "ps5")
#   -p PORT       FTP port (default: 2121)
#   -d DIST_DIR   directory that holds the built title (default: dist)
#   -t TITLE_ID   title to publish (default: the only one found)
#   -u USER       FTP user (default: anonymous)
#   -P PASSWORD   FTP password (default: the user name, then "codex")
#   --undeploy    remove the title folder from the console instead
#   --dry-run     print what would be transferred and change nothing

set -euo pipefail

host=${PS5_HOST:-ps5}
port=2121
dist_dir=dist
title_id=""
user=anonymous
password=""
action=deploy

while [[ $# -gt 0 ]]; do
    case $1 in
        -h) host=$2; shift 2 ;;
        -p) port=$2; shift 2 ;;
        -d) dist_dir=$2; shift 2 ;;
        -t) title_id=$2; shift 2 ;;
        -u) user=$2; shift 2 ;;
        -P) password=$2; shift 2 ;;
        --undeploy) action=undeploy; shift ;;
        --dry-run) dry_run=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

cd "$(dirname "$0")/../../.."

[[ -d ${dist_dir} ]] || {
    echo "${dist_dir} does not exist; build the title first:" >&2
    echo "  cmake -B cmake-build-ps5-title -DPLATFORM_PS5=ON -DPS5_NATIVE_TITLE=ON" >&2
    echo "  cmake --build cmake-build-ps5-title --target wiliwili.title" >&2
    exit 2
}

if [[ -z ${title_id} ]]; then
    # bash 3.2 has no mapfile, and this has to run on a stock macOS
    candidates=()
    while IFS= read -r candidate; do
        candidates+=("${candidate}")
    done < <(
        find "${dist_dir}" -mindepth 1 -maxdepth 1 -type d -exec test -f '{}/sce_sys/param.json' ';' -print 2>/dev/null | sort
    )
    if [[ ${#candidates[@]} -ne 1 ]]; then
        echo "expected exactly one title folder in ${dist_dir}, found ${#candidates[@]}; pass one with -t" >&2
        exit 2
    fi
    title_id=$(basename "${candidates[0]}")
fi

artifact="${dist_dir}/${title_id}"
[[ -s ${artifact}/eboot.bin && -s ${artifact}/sce_sys/param.json ]] || {
    echo "${artifact} is missing eboot.bin or sce_sys/param.json" >&2
    exit 2
}

if [[ -z ${password} ]]; then
    # ps5-payload-dev's ftpsrv accepts anonymous without a password, drakmor's
    # checks a fixed one. Both are tried below.
    password=codex
fi

printf '==> [title] %s: %s\n' "${action}" "${artifact}"
printf '==> [title] Console: %s:%s\n' "${host}" "${port}"

if [[ ${dry_run:-0} == 1 ]]; then
    echo "==> [title] Dry run complete; no network request was sent"
    exit 0
fi

python3 - "${action}" "${host}" "${port}" "${user}" "${password}" "${title_id}" "$(pwd)/${artifact}" <<'PY'
from ftplib import FTP, error_perm
from pathlib import Path
from posixpath import dirname, join
import sys

action, host, port, user, password, title_id, artifact_name = sys.argv[1:]
homebrew_root = "/data/homebrew"
artifact = Path(artifact_name)


def reply_code(error):
    return str(error).split(maxsplit=1)[0]


def login(ftp):
    try:
        ftp.login(user, password)
    except error_perm:
        if user != "anonymous":
            raise
        ftp.login(user, "")


def unlock_writes(ftp):
    """ps5-payload-dev's ftpsrv refuses writes until MTRW is sent.

    drakmor's ftpsrv needs no such command and answers 500, which is not an
    error here.
    """
    try:
        ftp.sendcmd("MTRW")
    except error_perm:
        pass


def list_names(ftp, path):
    previous = ftp.pwd()
    ftp.cwd(path)
    try:
        return {name for name, _ in ftp.mlsd() if name not in {".", ".."}}
    finally:
        ftp.cwd(previous)


def ensure_directory(ftp, path):
    current = ""
    for component in path.strip("/").split("/"):
        current += f"/{component}"
        try:
            ftp.mkd(current)
        except error_perm as error:
            if reply_code(error) != "550":
                raise
            previous = ftp.pwd()
            try:
                ftp.cwd(current)
            finally:
                ftp.cwd(previous)


def remove_if_present(ftp, path):
    try:
        ftp.sendcmd(f"DELE {path}")
        return True
    except error_perm as error:
        text = str(error).lower()
        if reply_code(error) == "550" and ("no such" in text or "not found" in text):
            return False
        raise


def remove_tree(ftp, path):
    try:
        ftp.sendcmd(f"DELE {path}")
        # ftpsrv implements DELE with POSIX remove(), which removes an empty
        # directory as well.
        return True
    except error_perm as error:
        text = str(error).lower()
        if reply_code(error) == "550" and ("no such" in text or "not found" in text):
            return False
        if reply_code(error) != "550" or ("directory not empty" not in text and "not a regular file" not in text and "is a directory" not in text):
            raise
    for name in list_names(ftp, path):
        if not name or "/" in name or "\r" in name or "\n" in name:
            raise RuntimeError(f"unsafe FTP entry name: {name!r}")
        remove_tree(ftp, join(path, name))
    ftp.rmd(path)
    return True


def upload(ftp, local, remote):
    """Store one file, atomically when the server supports rename."""
    directory = dirname(remote)
    ensure_directory(ftp, directory)
    temporary = join(directory, f".{remote.rsplit('/', 1)[-1]}.upload")
    with local.open("rb") as source:
        ftp.storbinary(f"STOR {temporary}", source, blocksize=256 * 1024)
    try:
        remove_if_present(ftp, remote)
        ftp.rename(temporary, remote)
    except error_perm:
        # No rename on this server: the finished file is stored directly.
        with local.open("rb") as source:
            ftp.storbinary(f"STOR {remote}", source, blocksize=256 * 1024)
        remove_if_present(ftp, temporary)


with FTP() as ftp:
    ftp.connect(host, int(port), timeout=30)
    login(ftp)
    unlock_writes(ftp)
    remote_root = join(homebrew_root, title_id)

    if action == "undeploy":
        if remove_tree(ftp, remote_root):
            print(f"==> [title] Removed {remote_root}")
        else:
            print(f"==> [title] Nothing to remove: {remote_root}")
    else:
        critical = [artifact / "eboot.bin", artifact / "sce_sys/param.json"]
        files = sorted(
            path for path in artifact.rglob("*")
            if path.is_file() and path not in critical
        ) + critical
        total = sum(path.stat().st_size for path in files)
        print(f"==> [title] Publishing {len(files)} files, {total / 1048576:.1f} MiB")
        for index, local in enumerate(files, 1):
            relative = local.relative_to(artifact).as_posix()
            if "\n" in relative or "\r" in relative:
                raise RuntimeError(f"deployment paths cannot contain newlines: {relative!r}")
            print(f"==> [title] [{index}/{len(files)}] {relative}")
            upload(ftp, local, join(remote_root, relative))
        if "eboot.bin" not in list_names(ftp, remote_root):
            raise RuntimeError("the upload finished but eboot.bin is not listed")
        if "param.json" not in list_names(ftp, join(remote_root, "sce_sys")):
            raise RuntimeError("the upload finished but param.json is not listed")
        print(f"==> [title] Published ftp://{host}:{port}{remote_root}/")
        print("==> [title] Reload the ShadowMountPlus payload to make the title appear")

    try:
        ftp.quit()
    except (EOFError, OSError):
        pass
PY

if [[ ${action} == "deploy" && -d "${HOME}/Downloads" ]]; then
    printf '==> [title] Copying to %s/Downloads/%s ...\n' "${HOME}" "${title_id}"
    rm -rf "${HOME}/Downloads/${title_id}"
    cp -a "${artifact}" "${HOME}/Downloads/${title_id}"
    if command -v zip >/dev/null 2>&1; then
        # The tag of this build, as the compiler saw it: the header is written
        # into the build directory, so the base version is not repeated here.
        # The build directory that made this folder is the one beside it - the
        # verification run publishes dist-citest from cmake-build-ps5-citest -
        # so the tag comes from there rather than from a build that is not the
        # one being published.
        build_tag=""
        build_dir=${PS5_TITLE_BUILD_DIR:-}
        if [[ -z ${build_dir} ]]; then
            dist_name=$(basename "${dist_dir}")
            if [[ ${dist_name} == "dist" ]]; then
                build_dir="cmake-build-ps5-title"
            else
                build_dir="cmake-build-ps5-${dist_name#dist-}"
            fi
        fi
        header="${build_dir}/include/build_version.h"
        if [[ -f ${header} ]]; then
            build_tag=$(sed -n 's/.*#define WILIWILI_BUILD_VERSION_TAG "\(.*\)"/\1/p' "${header}" | head -1)
        fi
        (
            cd "${dist_dir}"
            rm -f "${HOME}/Downloads/${title_id}.zip" "${HOME}/Downloads/wiliwili-PS5-v1.6.0.zip" "${HOME}/Downloads/wiliwili-PS5-v1.6.0.1.zip" "${HOME}/Downloads/wiliwili-PS5-v1.6.0.5.zip"
            zip -rq -X "${HOME}/Downloads/${title_id}.zip" "${title_id}"
            cp -f "${HOME}/Downloads/${title_id}.zip" "${HOME}/Downloads/wiliwili-PS5-v1.6.0.5.zip"
            if [[ -n "${build_tag}" ]]; then
                cp -f "${HOME}/Downloads/${title_id}.zip" "${HOME}/Downloads/wiliwili-PS5-${build_tag}.zip"
            fi
        )
        printf '==> [title] Created %s/Downloads/%s.zip\n' "${HOME}" "${title_id}"
        printf '==> [title] Created %s/Downloads/wiliwili-PS5-v1.6.0.5.zip\n' "${HOME}"
        if [[ -n "${build_tag}" ]]; then
            printf '==> [title] Created %s/Downloads/wiliwili-PS5-%s.zip\n' "${HOME}" "${build_tag}"
        fi

        # Package into ffpfsc format using ps5-dump-forge
        package_script="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/package_ffpfs.sh"
        if [[ -f "${package_script}" ]]; then
            pkg_name=""
            if [[ -n "${build_tag}" ]]; then
                pkg_name="wiliwili-PS5-${build_tag}"
            fi
            bash "${package_script}" -i "${artifact}" -o "${HOME}/Downloads" ${pkg_name:+-n "${pkg_name}"}
        fi
    fi
fi
