#!/usr/bin/env bash
# wiliwili - PS5 native title: build ps5-native-tool, the converter that turns a
# linked ELF into the module format the console loads.
#
# The tool is built from the ps5-native-app-boilerplate checkout (PS5_BOILERPLATE)
# with two local notes:
#
#   * the process parameters carry the loader heap. The default is the
#     ps5-opengl native-app recipe the OpenGL runtime expects: a bounded 256 MiB
#     heap (ps5-opengl patches the boilerplate the same way before it builds its
#     native test and CTS applications, and ProsperoLight ships the same value).
#     PS5_TITLE_HEAP_SIZE overrides it; the boilerplate's own baseline is
#     0xffffffffffffffff ("as much as possible").
#   * zlib is taken from the host instead of the boilerplate's pinned private
#     copy: it only computes container checksums, and the module's own geometry
#     does not depend on the zlib version.
#
# Required environment:
#   PS5_BOILERPLATE     ps5-native-app-boilerplate checkout
# Optional:
#   PS5_NATIVE_TOOL     output path (default <checkout>/build-host/ps5-native-tool)
#   PS5_TITLE_HEAP_SIZE loader heap in bytes (default 0x10000000)
#   PS5_ZLIB_INCLUDE    include directory of a private zlib
#   PS5_ZLIB_LIB        static zlib archive to link instead of the host -lz
#   CXX                 host C++ compiler (default: clang++-18, then clang++/c++)

set -euo pipefail

: "${PS5_BOILERPLATE:?set PS5_BOILERPLATE to a ps5-native-app-boilerplate checkout}"

heap_size=${PS5_TITLE_HEAP_SIZE:-0x10000000}
tool=${PS5_NATIVE_TOOL:-$PS5_BOILERPLATE/build-host/ps5-native-tool}
native=$PS5_BOILERPLATE/tooling/native
writer=$native/sce_module_writer.cpp

[[ -f $writer ]] || {
    echo "'$writer' is missing; PS5_BOILERPLATE does not look like the boilerplate" >&2
    exit 2
}

if [[ -z ${CXX:-} ]]; then
    for candidate in clang++-18 /opt/homebrew/opt/llvm@18/bin/clang++ clang++ c++; do
        if command -v "$candidate" >/dev/null 2>&1; then
            CXX=$(command -v "$candidate")
            break
        fi
    done
fi
[[ -n ${CXX:-} ]] || {
    echo "no host C++ compiler found; set CXX" >&2
    exit 2
}

# The field is written as a literal in the parameter block builder. A checkout
# that already carries the wanted value is accepted as it is.
heap_default='write_u64(result.data, result.heap_size, std::numeric_limits<std::uint64_t>::max());'
if [[ $(grep -Fc "$heap_default" "$writer") == 1 ]]; then
    echo "==> [title] Setting the loader heap of ps5-native-tool to $heap_size"
    sed -i.bak "s|$heap_default|write_u64(result.data, result.heap_size, $heap_size);|" "$writer"
    rm -f "$writer.bak"
elif [[ $(grep -Fc "write_u64(result.data, result.heap_size, $heap_size);" "$writer") == 1 ]]; then
    echo "==> [title] ps5-native-tool already asks for a $heap_size loader heap"
else
    echo "the heap parameter of '$writer' is neither the boilerplate default nor $heap_size;" >&2
    echo "check how it changed before patching" >&2
    exit 2
fi

zlib_include=()
zlib_link=(-lz)
if [[ -n ${PS5_ZLIB_INCLUDE:-} ]]; then
    zlib_include=(-I "$PS5_ZLIB_INCLUDE")
fi
if [[ -n ${PS5_ZLIB_LIB:-} ]]; then
    zlib_link=("$PS5_ZLIB_LIB")
fi

mkdir -p "$(dirname -- "$tool")"
echo "==> [title] Building $tool with $CXX"
# ${x[@]+...} keeps an empty array from tripping set -u on bash 3.2, the
# version macOS ships.
"$CXX" -std=c++20 -O2 -Wall -Wextra -Werror ${zlib_include[@]+"${zlib_include[@]}"} \
    "$native/native_app_builder.cpp" "$native/self_container.cpp" \
    "$native/elf_object.cpp" "$native/sce_module_writer.cpp" \
    ${zlib_link[@]+"${zlib_link[@]}"} -o "$tool"

"$tool" --help > /dev/null 2>&1 || true
[[ $("$tool" --help 2>&1 || true) == *ps5-native-tool* ]] || {
    echo "the freshly built tool does not answer --help" >&2
    exit 2
}
printf 'ps5-native-tool: %s\n' "$tool"
sha256sum "$tool" || shasum -a 256 "$tool"
