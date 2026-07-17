#!/usr/bin/env python3
from __future__ import annotations

import csv
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        temp = Path(tmp)

        raw = temp / "overlay.bin"
        words = [0x27BDFFE0, 0xAFBF001C, 0x0C006000, 0, 0x03E00008, 0]
        raw.write_bytes(b"".join(struct.pack("<I", word) for word in words))
        scan = subprocess.check_output(
            [
                sys.executable,
                str(ROOT / "scripts" / "scan_mips.py"),
                str(raw),
                "--base",
                "0x80010000",
            ],
            text=True,
        )
        assert "stack_frame_prologues" in scan
        assert "jr_ra" in scan

        symbols = temp / "symbols.csv"
        symbols.write_text(
            "name,address,kind,size,comment,source,confidence\n"
            "Foo Bar,0x80010000,function,32,test,sym,exact\n",
            encoding="utf-8",
        )
        output = temp / "symbols.rz"
        subprocess.check_call(
            [
                sys.executable,
                str(ROOT / "scripts" / "symbols_to_rizin.py"),
                str(symbols),
                "-o",
                str(output),
            ]
        )
        generated = output.read_text(encoding="utf-8")
        assert "afn Foo_Bar @ 0x80010000" in generated

        matrix = temp / "replays.csv"
        source_template = ROOT / "templates" / "replay-matrix.csv"
        header = source_template.read_text(encoding="utf-8").splitlines()[0]
        matrix.write_text(
            header
            + "\n"
            + "demo01,demo.bin,abc,passed,BizHawk,2.x,Octoshock,disc,bios,NTSC,boot,pad,default,0,100,event,ovl1,func1,func1,0x80010000,trace.jsonl,ok\n",
            encoding="utf-8",
        )
        report = temp / "report.md"
        subprocess.check_call(
            [
                sys.executable,
                str(ROOT / "scripts" / "replay_coverage.py"),
                str(matrix),
                "--report",
                str(report),
            ]
        )
        assert "Total scenarios: **1**" in report.read_text(encoding="utf-8")

    print("test_helpers: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
