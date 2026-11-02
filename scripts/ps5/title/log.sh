#!/usr/bin/env bash
# wiliwili - PS5 native title: fetch what the console recorded about a run.
#
# A title's output has no reader, so the application redirects it into
# /data/wiliwili/wiliwili-title.log. The loader and the system print their own
# messages to the kernel log, which the console serves on port 3232 while a
# klog payload is running.
#
# Usage:
#   scripts/ps5/title/log.sh [-h HOST] [-p PORT] [-k SECONDS] [-o FILE] [-f]
#
# Options:
#   -h HOST       console address (default: $PS5_HOST or "ps5")
#   -p PORT       FTP port of the console (default: 2121), the application log
#   -k SECONDS    also read the kernel log for this long (default: 3, 0 to skip)
#   -o FILE       save the application log to FILE (default: only print it)
#   -f            follow the kernel log until interrupted (Ctrl-C)
#
# The kernel log (port 3232, klogsrv) carries the title's warnings and errors as
# they happen, and the loader's own lines. The file log keeps everything,
# including the debug lines, and is what to look at once the title has stopped.

set -euo pipefail

host=${PS5_HOST:-ps5}
port=2121
klog_seconds=3
output=""
follow=0

while getopts "h:p:k:o:f" opt; do
    case ${opt} in
        h) host=$OPTARG ;;
        p) port=$OPTARG ;;
        k) klog_seconds=$OPTARG ;;
        o) output=$OPTARG ;;
        f) follow=1 ;;
        *) echo "usage: $0 [-h HOST] [-p PORT] [-k SECONDS] [-o FILE] [-f]" >&2; exit 2 ;;
    esac
done

echo "==> [title] /data/wiliwili/wiliwili-title.log on ${host}" >&2
fetch_log() {
    # Read the file over FTP into a temp file first. Capturing curl in a
    # command substitution truncates the stream in some configurations.
    local tmp
    tmp=$(mktemp "${TMPDIR:-/tmp}/wiliwili-fetch.XXXXXX") || return 1
    if ! curl --silent --show-error --fail --max-time 30 \
        "ftp://${host}:${port}/data/wiliwili/wiliwili-title.log" -o "${tmp}"; then
        rm -f "${tmp}"
        echo "the log could not be read; the title may never have started" >&2
        return 1
    fi
    printf '%s\n' "${tmp}"
}
tmp_log=$(fetch_log) || exit 1
echo "## wiliwili-title.log ($(wc -c < "${tmp_log}") bytes)" >&2
if [[ ${output} == "" ]]; then
    cat "${tmp_log}"
else
    cp "${tmp_log}" "${output}"
    echo "==> [title] Saved to ${output} ($(wc -c < "${output}") bytes)" >&2
fi

# The crash report (scripts/ps5/title/crash_report.cpp) is written beside the
# log when the application faulted; symbolize it with
# scripts/ps5/title/symbolize_crash.py and the build's title/llvm-pie.elf.
crash=$(curl --silent --show-error --max-time 30 \
    "ftp://${host}:${port}/data/wiliwili/crash-last.txt") || true
if [[ -n ${crash} && ${crash} != *"Failed to open file"* ]]; then
    echo "==> [title] /data/wiliwili/crash-last.txt on ${host}" >&2
    printf '%s\n' "${crash}"
    if [[ -n ${output} ]]; then
        printf '%s\n' "${crash}" > "${output%.*}.crash.txt"
        echo "==> [title] Saved to ${output%.*}.crash.txt" >&2
    fi
fi

# The logger probes (library/borealis/library/include/borealis/core/logger.hpp)
# that wrote logprobe.log came from a diagnostic build and are gone; the shim
# deletes the leftover file at start-up.

if [[ ${follow} == 1 ]]; then
    echo "==> [title] Kernel log of ${host}:3232, Ctrl-C to stop" >&2
    if [[ -n ${output} ]]; then
        exec nc "${host}" 3232 2>/dev/null | tee -a "${output%.*}.klog.txt"
    fi
    exec nc "${host}" 3232 2>/dev/null
fi

if [[ ${klog_seconds} != 0 ]]; then
    echo "==> [title] Kernel log of ${host}:3232 (${klog_seconds}s)" >&2
    if command -v nc > /dev/null; then
        if [[ -n ${output} ]]; then
            nc -w "${klog_seconds}" "${host}" 3232 2>/dev/null | tee "${output%.*}.klog.txt" | tail -n 60 || true
            echo "==> [title] Saved to ${output%.*}.klog.txt" >&2
        else
            nc -w "${klog_seconds}" "${host}" 3232 2>/dev/null | tail -n 60 || true
        fi
    else
        echo "nc is not installed, skipping the kernel log" >&2
    fi
fi
