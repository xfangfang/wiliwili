#!/usr/bin/env python3
"""Attach to a pid via ps5debug-NG, dump thread registers, detach.

Usage: python3 dbg_live.py -H <console-ip> <pid>
The server connects back to port 755 for async debug interrupts.
"""
import argparse
import socket
import struct
import sys
import threading

sys.path.insert(0, __import__("os").path.dirname(__file__))
from dbg import (cmd, recv_exact, recv_status,  # noqa: E402
                 CMD_DEBUG_ATTACH, CMD_DEBUG_DETACH, CMD_DEBUG_GET_THREAD_LIST,
                 CMD_DEBUG_GETREGS, ST_SUCCESS)

NAMES = ["r15", "r14", "r13", "r12", "r11", "r10", "r9", "r8", "rdi", "rsi",
         "rbp", "rbx", "rdx", "rax", "rcx", "rsp", "rip", "eflags"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-H", "--host", required=True)
    ap.add_argument("pid", type=int)
    ns = ap.parse_args()

    # Accept the interrupt connection before attach.
    intr = []
    srv = socket.socket()
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("", 755))
    srv.listen(1)
    acceptor = threading.Thread(target=lambda: intr.append(srv.accept()[0]))
    acceptor.start()

    s = socket.create_connection((ns.host, 744), timeout=30)
    st = cmd(s, CMD_DEBUG_ATTACH, struct.pack("<I", ns.pid))
    print("attach:", "ok" if st == ST_SUCCESS else hex(st))
    if st != ST_SUCCESS:
        return 1

    acceptor.join(5)
    if intr:
        print("interrupt connection open (755)")
    else:
        srv.close()

    st = cmd(s, CMD_DEBUG_GET_THREAD_LIST)
    assert st == ST_SUCCESS, hex(st)
    (num,) = struct.unpack("<I", recv_exact(s, 4))
    tids = struct.unpack(f"<{num}I", recv_exact(s, 4 * num)) if num else ()
    print("threads:", tids)

    for tid in tids[:8]:
        st = cmd(s, CMD_DEBUG_GETREGS, struct.pack("<I", tid))
        if st != ST_SUCCESS:
            print(f"tid {tid}: regs status {hex(st)}")
            continue
        raw = recv_exact(s, 176)
        vals = struct.unpack("<17QI", raw)
        d = dict(zip(NAMES, vals))
        print(f"tid {tid}: rip={d['rip']:x} rsp={d['rsp']:x} "
              f"rbp={d['rbp']:x} rax={d['rax']:x} rdi={d['rdi']:x} rsi={d['rsi']:x}")

    sin = socket.create_connection((ns.host, 744), timeout=30)
    st = cmd(sin, CMD_DEBUG_DETACH)
    print("detach:", "ok" if st == ST_SUCCESS else hex(st))

    if intr:
        intr[0].settimeout(2)
        try:
            data = intr[0].recv(4096)
            print("interrupt data:", data[:64].hex())
        except OSError:
            pass
        intr[0].close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
