#!/usr/bin/env python3
from __future__ import annotations

import json
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "psx_exe.py"


def make_exe(path: Path) -> bytes:
    header = bytearray(0x800)
    header[:8] = b"PS-X EXE"
    fields = {
        0x10: 0x80010010,
        0x14: 0x80020000,
        0x18: 0x80010000,
        0x1C: 0x20,
        0x20: 0,
        0x24: 0,
        0x28: 0x80030000,
        0x2C: 0x100,
        0x30: 0x801FFF00,
        0x34: 0,
    }
    for offset, value in fields.items():
        struct.pack_into("<I", header, offset, value)
    payload = bytes(range(0x20))
    path.write_bytes(header + payload)
    return payload


def run(*args: str) -> str:
    return subprocess.check_output([sys.executable, str(SCRIPT), *args], text=True).strip()


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        temp = Path(tmp)
        exe = temp / "GAME.EXE"
        expected = make_exe(exe)

        info = json.loads(run("inspect", str(exe), "--json"))
        assert info["text_address"] == 0x80010000
        assert info["text_size"] == 0x20
        assert info["initial_pc"] == 0x80010010

        assert run("offset-to-addr", str(exe), "0x810") == "0x80010010"
        assert run("addr-to-offset", str(exe), "0x80010010") == "0x810"

        output = temp / "payload.bin"
        run("extract", str(exe), "-o", str(output))
        assert output.read_bytes() == expected

        aliases = json.loads(run("aliases", "0xa0012340"))
        assert aliases["physical_candidate"] == "0x00012340"
        assert aliases["cached_kseg0_candidate"] == "0x80012340"

    print("test_psx_exe: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
