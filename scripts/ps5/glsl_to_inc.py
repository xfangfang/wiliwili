#!/usr/bin/env python3
"""把 GLSL 片元着色器编译成 nanovg 用的 uint32_t SPIR-V 数组。

nanovg 的 Vulkan 后端（library/borealis/.../nanovg/nanovg_vk.h）是把 SPIR-V
直接以 C 数组的形式内嵌进去的：

    const uint32_t fillFragShader[] = {
    #include "shader/fill.frag.inc"
    };

所以只改 `shader/fill_base.frag` 而不同步 `.inc`，运行期不会有任何变化
（编译能过，画面纹丝不动）。改完着色器要跑一次：

    scripts/ps5/glsl_to_inc.py shader/fill_base.frag shader/fill.frag.inc
    scripts/ps5/glsl_to_inc.py -D EDGE_AA shader/fill_base.frag shader/fill_edge_aa.frag.inc

需要 glslc（macOS: brew install shaderc，Debian/Ubuntu: apt install glslc）。
"""
import argparse
import os
import struct
import subprocess
import sys
import tempfile

HEADER = "\t// Generated SPIR-V with HDR PQ, HLG and Dolby Vision Profile 5"
WORDS_PER_LINE = 8


def compile_shader(source, defines, glslc):
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "shader.spv")
        cmd = [glslc, "--target-env=vulkan1.0", "-fshader-stage=frag"]
        cmd += ["-D" + d for d in defines]
        cmd += ["-o", out, source]
        subprocess.run(cmd, check=True)
        with open(out, "rb") as fp:
            data = fp.read()
    if len(data) % 4:
        sys.exit("glslc 输出的 SPIR-V 不是 4 字节对齐的")
    return struct.unpack("<%dI" % (len(data) // 4), data)


def write_inc(words, target):
    lines = [HEADER]
    for i in range(0, len(words), WORDS_PER_LINE):
        chunk = "".join("0x%08x," % w for w in words[i : i + WORDS_PER_LINE])
        lines.append("\t" + chunk)
    with open(target, "w") as fp:
        fp.write("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", help="GLSL 片元着色器")
    parser.add_argument("target", help="要写入的 .inc 文件")
    parser.add_argument("-D", dest="defines", action="append", default=[], help="传给 glslc 的宏，可重复")
    parser.add_argument("--glslc", default="glslc", help="glslc 可执行文件路径")
    args = parser.parse_args()

    words = compile_shader(args.source, args.defines, args.glslc)
    write_inc(words, args.target)
    print("%s -> %s (%d SPIR-V words)" % (args.source, args.target, len(words)))


if __name__ == "__main__":
    main()
