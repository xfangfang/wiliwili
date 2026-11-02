#!/usr/bin/env python3
"""Minimal ps5debug-NG client (wire-compatible with Ctn ps5debug 1.0b5).

Usage:
  dbg.py -h <host> version
  dbg.py -h <host> ping
  dbg.py -h <host> proclist
  dbg.py -h <host> maps    <pid>
  dbg.py -h <host> read    <pid> <addr-hex> <length-hex>
Protocol: https://github.com/Pharaoh2k/ps5debug-NG (PROTOCOL.md)
"""
import argparse
import socket
import struct
import sys

PACKET_MAGIC = 0xFFAABBCC

CMD_VERSION = 0xBD000001
CMD_PROC_NOP = 0xBDAACC06
CMD_PROC_LIST = 0xBDAA0001
CMD_PROC_READ = 0xBDAA0002
CMD_PROC_MAPS = 0xBDAA0004
CMD_PROC_SCAN_AOB = 0xBDAA0501
CMD_PROC_INFO = 0xBDAA000A
CMD_DEBUG_ATTACH = 0xBDBB0001
CMD_DEBUG_DETACH = 0xBDBB0002
CMD_DEBUG_GET_THREAD_LIST = 0xBDBB0005
CMD_DEBUG_GETREGS = 0xBDBB0008

ST_SUCCESS = 0x80000000  # post-bitswap CMD_SUCCESS
ST_ERROR = 0xF0000001
ST_DATA_NULL = 0xF0000003

CMD_PROC_AUTH = 0xBDAACCFF
AUTH_MAGIC = 0xBB40E64D


def recv_exact(s, n):
    buf = b""
    while len(buf) < n:
        chunk = s.recv(n - len(buf))
        if not chunk:
            raise EOFError(f"connection closed after {len(buf)}/{n} bytes")
        buf += chunk
    return buf


def recv_status(s):
    (st,) = struct.unpack("<I", recv_exact(s, 4))
    return st


def cmd(s, cid, body=b""):
    s.sendall(struct.pack("<III", PACKET_MAGIC, cid, len(body)) + body)
    return recv_status(s)


def _lfsr_next(state):
    s1, s2, s3, s4 = [x & 0xFFFFFFFF for x in state]
    s1 = ((s1 << 18) & 0xFFF80000) ^ (((s1 ^ ((s1 << 6) & 0xFFFFFFFF)) >> 13))
    s2 = ((s2 << 2) & 0xFFFFFFE0) ^ (((s2 ^ ((s2 << 2) & 0xFFFFFFFF)) >> 27))
    s3 = ((s3 << 7) & 0xFFFFF800) ^ (((s3 ^ ((s3 << 13) & 0xFFFFFFFF)) >> 21))
    s4 = ((s4 << 13) & 0xFFF00000) ^ (((s4 ^ ((s4 << 3) & 0xFFFFFFFF)) >> 12))
    s1 &= 0xFFFFFFFF
    s2 &= 0xFFFFFFFF
    s3 &= 0xFFFFFFFF
    s4 &= 0xFFFFFFFF
    state[:] = [s1, s2, s3, s4]
    return s1 ^ s2 ^ s3 ^ s4


def auth(s, flags=2):
    """PROTOCOL 1.8 handshake. flags&2 enables the stateful scan trio."""
    body = struct.pack("<II", AUTH_MAGIC, flags)
    st = cmd(s, CMD_PROC_AUTH, body)
    assert st == ST_SUCCESS, hex(st)
    (chlen,) = struct.unpack("<H", recv_exact(s, 2))
    challenge = bytearray(recv_exact(s, chlen))
    state = [200, 300, 400, 500]
    for i in range(chlen):
        challenge[i] ^= _lfsr_next(state) & 0xFF
    s.sendall(bytes(challenge))
    return recv_status(s)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-H", "--host", required=True)
    ap.add_argument("-p", "--port", type=int, default=744)
    ap.add_argument("cmdname")
    ap.add_argument("args", nargs="*")
    ns = ap.parse_args()

    s = socket.create_connection((ns.host, ns.port), timeout=120)
    try:
        if ns.cmdname == "version":
            # No status word precedes the payload.
            s.sendall(struct.pack("<III", PACKET_MAGIC, CMD_VERSION, 0))
            (n,) = struct.unpack("<I", recv_exact(s, 4))
            print("version:", recv_exact(s, n).decode())

        elif ns.cmdname == "ping":
            st = cmd(s, CMD_PROC_NOP)
            print("ping:", "ok" if st == ST_SUCCESS else hex(st))

        elif ns.cmdname == "proclist":
            st = cmd(s, CMD_PROC_LIST)
            assert st == ST_SUCCESS, hex(st)
            (num,) = struct.unpack("<I", recv_exact(s, 4))
            for _ in range(num):
                raw = recv_exact(s, 36)
                name = raw[:32].split(b"\0")[0].decode(errors="replace")
                (pid,) = struct.unpack("<i", raw[32:])
                print(f"{pid:>10}  {name}")

        elif ns.cmdname == "maps":
            raise SystemExit  # use "procmaps"

        elif ns.cmdname == "procmaps":
            pid = int(ns.args[0])
            st = cmd(s, CMD_PROC_MAPS, struct.pack("<I", pid))
            assert st == ST_SUCCESS, hex(st)
            (num,) = struct.unpack("<I", recv_exact(s, 4))
            for _ in range(num):
                raw = recv_exact(s, 58)
                name = raw[:32].split(b"\0")[0].decode(errors="replace")
                start, end, off = struct.unpack("<QQQ", raw[32:56])
                (prot,) = struct.unpack("<H", raw[56:58])
                print(f"{start:012x}-{end:012x} off={off:012x} prot={prot:04x} {name}")

        elif ns.cmdname == "read":
            pid = int(ns.args[0])
            addr = int(ns.args[1], 16)
            length = int(ns.args[2], 16)
            out = open(ns.args[3], "wb") if len(ns.args) > 3 else None
            st = cmd(s, CMD_PROC_READ, struct.pack("<IqI", pid, addr, length))
            assert st == ST_SUCCESS, hex(st)
            remaining = length
            data = b""
            while remaining > 0:
                chunk = recv_exact(s, min(remaining, 0x10000))
                data += chunk
                remaining -= len(chunk)
            if out:
                out.write(data)
                out.close()
                print(f"read {len(data)} bytes -> {ns.args[3]}")
            else:
                print(data.hex())

        elif ns.cmdname == "scan":
            # scan <pid> <addr-hex> <length-hex> <pattern-hex> [--auth]
            pid = int(ns.args[0])
            addr = int(ns.args[1], 16)
            length = int(ns.args[2], 16)
            pattern = bytes.fromhex(ns.args[3])
            authflag = len(ns.args) > 4 and ns.args[4] == "auth"
            if authflag:
                st2 = auth(s, 3)
                print("auth:", "ok" if st2 == ST_SUCCESS else hex(st2))
            body = struct.pack("<IqIBBI", pid, addr, length, 1, 0, len(pattern))
            s.sendall(struct.pack("<III", PACKET_MAGIC, CMD_PROC_SCAN_AOB, len(body)) + body)
            for _ in range(2):
                assert recv_status(s) == ST_SUCCESS
            (match,) = struct.unpack("<Q", recv_exact(s, 8))
            assert recv_status(s) == ST_SUCCESS
            print(hex(match) if match else "not found")

        elif ns.cmdname == "info":
            pid = int(ns.args[0])
            st = cmd(s, CMD_PROC_INFO, struct.pack("<I", pid))
            assert st == ST_SUCCESS, hex(st)
            raw = recv_exact(s, 188)
            pid2, name, path, titleid, contentid = struct.unpack("<I40s64s16s64s", raw)
            fmt = lambda x: x.split(b"\0")[0].decode(errors="replace")
            print(f"pid={pid2} name={fmt(name)}")
            print(f"path={fmt(path)}")
            print(f"titleid={fmt(titleid)} contentid={fmt(contentid)}")

        elif ns.cmdname == "attach":
            pid = int(ns.args[0])
            st = cmd(s, CMD_DEBUG_ATTACH, struct.pack("<I", pid))
            print("attach:", "ok" if st == ST_SUCCESS else hex(st))

        elif ns.cmdname == "detach":
            st = cmd(s, CMD_DEBUG_DETACH)
            print("detach:", "ok" if st == ST_SUCCESS else hex(st))

        elif ns.cmdname == "threads":
            st = cmd(s, CMD_DEBUG_GET_THREAD_LIST)
            assert st == ST_SUCCESS, hex(st)
            (num,) = struct.unpack("<I", recv_exact(s, 4))
            ids = struct.unpack(f"<{num}I", recv_exact(s, 4 * num)) if num else ()
            print(" ".join(map(str, ids)))

        elif ns.cmdname == "regs":
            lwpid = int(ns.args[0])
            st = cmd(s, CMD_DEBUG_GETREGS, struct.pack("<I", lwpid))
            assert st == ST_SUCCESS, hex(st)
            raw = recv_exact(s, 176)
            # FreeBSD amd64 struct reg: r15..r11, rdi, rsi, rbp, rbx, rdx,
            # rax, rcx, rsp, rip, eflags
            names = ["r15", "r14", "r13", "r12", "r11", "r10", "r9", "r8",
                     "rdi", "rsi", "rbp", "rbx", "rdx", "rax", "rcx", "rsp",
                     "rip", "eflags"]
            vals = struct.unpack("<17QI", raw)
            for n_, v_ in zip(names, vals):
                print(f"{n_:7} {v_:016x}")
        else:
            ap.error(f"unknown command {ns.cmdname}")
    finally:
        s.close()


if __name__ == "__main__":
    main()
