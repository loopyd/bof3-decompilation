#!/usr/bin/env python3
"""Discover proper functions across BOF3 binaries with rizin.

For each Splat code segment we strip its leading file bytes (so the code lands
at the segment's declared vram) and run rizin `aaa` to auto-detect function
boundaries. Detected functions are diffed against the subsegments already
declared in the target's Splat YAML.

Because the PS-X/EMI header offsets the binary, rizin must analyze a temp file
with the header removed; file_offset = seg.start + (vram - seg.vram).

Usage:
    python3 tools/python/rizin_discover.py            # report gaps
    python3 tools/python/rizin_discover.py --apply    # rewrite YAMLs
    python3 tools/python/rizin_discover.py --target emi/etc/game/00
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent.parent
SPLAT_DIR = ROOT / "config" / "splat"


@dataclass
class CodeSegment:
    name: str
    start: int
    vram: int
    end: int


@dataclass
class DeclaredSub:
    offset: int
    vram: int
    kind: str
    name: str


def _to_int(x: object) -> int:
    if isinstance(x, int):
        return x
    if isinstance(x, str):
        return int(x, 0)
    return 0


def load_code_segments(yaml_path: Path) -> list[CodeSegment]:
    doc = yaml.safe_load(yaml_path.read_text(encoding="utf-8"))
    segments = doc.get("segments", [])
    # Collect start offsets from ALL top-level segments (list or dict form) so a
    # code segment ends at the next top-level segment, not the binary tail.
    top_starts: list[int] = []
    for s in segments:
        if isinstance(s, dict):
            top_starts.append(_to_int(s.get("start", 0)))
        elif isinstance(s, list) and s:
            top_starts.append(_to_int(s[0]))
    top_starts = sorted(set(top_starts))
    out: list[CodeSegment] = []
    for s in segments:
        if not isinstance(s, dict) or s.get("type") != "code":
            continue
        start = _to_int(s["start"])
        vram = _to_int(s["vram"])
        end = None
        for ts in top_starts:
            if ts > start:
                end = ts
                break
        if end is None:
            end = len(yaml_path.read_bytes())
        out.append(CodeSegment(s.get("name", "code"), start, vram, end))
    return out


def load_declared(yaml_path: Path) -> list[DeclaredSub]:
    doc = yaml.safe_load(yaml_path.read_text(encoding="utf-8"))
    out: list[DeclaredSub] = []
    for s in doc.get("segments", []):
        if not isinstance(s, dict):
            continue
        seg_start = _to_int(s.get("start", 0))
        seg_vram = _to_int(s.get("vram", 0))
        for sub in s.get("subsegments", []):
            off = _to_int(sub[0])
            kind = sub[1]
            name = sub[2] if len(sub) > 2 else ""
            out.append(DeclaredSub(off, seg_vram + (off - seg_start), kind, name))
    return out


def rizin_functions(binary: Path, seg: CodeSegment, seed_starts: list[int] | None = None) -> list[dict]:
    """Strip header up to seg.start, map at seg.vram, analyze, return functions.

    rizin's autonomous `aaa` under-detects on small overlays (it skips functions
    between the entry and the first large function). Seeding `af` at every
    declared subsegment start forces analysis of those functions and lets `aac`
    follow the call graph into the gaps, yielding reliable boundaries.
    """
    raw = binary.read_bytes()
    tail = raw[seg.start:seg.end]
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td) / "code.bin"
        tmp.write_bytes(tail)
        vram = seg.vram
        setup = ["e scr.color=0", "aa"]
        for addr in seed_starts or []:
            setup.append(f"af @ {hex(addr)}")
        setup.append("aac")
        cmd = [
            "rizin", "-q0", "-N", "-n",
            "-a", "mips", "-b", "32", "-e", "cfg.bigendian=false",
            "-m", hex(vram),
            *(f"-c {c}" for c in setup),
            "-c", "aflj",
            str(tmp),
        ]
        try:
            res = subprocess.run(cmd, capture_output=True, text=True, timeout=400, cwd=ROOT)
        except subprocess.TimeoutExpired:
            print(f"  WARN: rizin timed out for {binary} seg {seg.name}", file=sys.stderr)
            return []
        arr = None
        for line in res.stdout.splitlines():
            line = line.strip()
            if line.startswith("["):
                try:
                    arr = json.loads(line)
                except json.JSONDecodeError:
                    pass
        if not isinstance(arr, list):
            return []
        funcs = []
        for f in arr:
            addr = int(f.get("offset", 0))
            size = int(f.get("size", 0))
            funcs.append({"vram": addr, "size": size})
        return funcs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--target")
    ap.add_argument("--debug", action="store_true")
    args = ap.parse_args()

    total_added = 0
    for yp in sorted(SPLAT_DIR.rglob("*.yaml")):
        rel = yp.relative_to(SPLAT_DIR).with_suffix("")
        target_id = str(rel).replace("\\", "/")
        if args.target and target_id != args.target:
            continue
        doc = yaml.safe_load(yp.read_text(encoding="utf-8"))
        binary_rel = doc.get("options", {}).get("target_path")
        if not binary_rel:
            continue
        binary = ROOT / binary_rel
        if not binary.is_file():
            continue

        declared = load_declared(yp)
        declared_starts = {d.vram for d in declared}
        code_segs = load_code_segments(yp)
        for seg in code_segs:
            if seg.end <= seg.start:
                seg.end = len(binary.read_bytes())

        # Build new subsegment lists per code segment.
        new_segments: list = []
        changed = False
        for s in doc.get("segments", []):
            if not isinstance(s, dict) or s.get("type") != "code":
                new_segments.append(s)
                continue
            seg_start = _to_int(s["start"])
            seg_vram = _to_int(s["vram"])
            seg = next(c for c in code_segs if c.start == seg_start)
            seed_starts = [d.vram for d in in_seg_decl]
            detected = rizin_functions(binary, seg, seed_starts)

            in_seg_decl = sorted(
                [d for d in declared if seg.vram <= d.vram < seg.vram + (seg.end - seg.start)],
                key=lambda d: d.offset,
            )
            if args.debug:
                print(f"  DEBUG seg {seg.name}: detected={len(detected)} declared={len(in_seg_decl)} "
                      f"asm={sum(1 for d in in_seg_decl if d.kind=='asm')}")
            c_extents: list[tuple[int, int]] = []
            asm_extents: list[tuple[int, int]] = []
            for i, d in enumerate(in_seg_decl):
                nxt = in_seg_decl[i + 1].vram if i + 1 < len(in_seg_decl) else seg.vram + (seg.end - seg.start)
                if d.kind == "c":
                    c_extents.append((d.vram, nxt))
                elif d.kind == "asm":
                    asm_extents.append((d.vram, nxt))

            boundaries: list[tuple[int, str, str]] = []
            for d in in_seg_decl:
                boundaries.append((d.vram, d.kind, d.name))
            for fn in detected:
                addr = fn["vram"]
                if any(a <= addr < b for a, b in asm_extents) and addr not in {b[0] for b in boundaries}:
                    boundaries.append((addr, "asm", f"func_{addr:08x}"))
            boundaries = sorted({b for b in boundaries}, key=lambda b: b[0])
            boundaries = [b for b in boundaries if b[1] == "c" or not any(c <= b[0] < e for c, e in c_extents)]

            new_subs = [[seg_start + (vram - seg_vram), kind, name] for vram, kind, name in boundaries]
            old_subs = [[d.offset, d.kind, d.name] for d in in_seg_decl]
            if new_subs != old_subs:
                changed = True
            s = dict(s)
            s["subsegments"] = new_subs
            new_segments.append(s)

        if changed:
            before_asm = sum(1 for d in declared if d.kind == "asm")
            after_asm = sum(1 for ns in new_segments if isinstance(ns, dict)
                            for sub in ns.get("subsegments", []) if sub[1] == "asm")
            added = after_asm - before_asm
            total_added += max(added, 0)
            if args.apply:
                doc = dict(doc)
                doc["segments"] = new_segments
                yp.write_text(yaml.safe_dump(doc, sort_keys=False, default_flow_style=False), encoding="utf-8")
            print(f"[{'apply' if args.apply else 'report'}] {target_id}: "
                  f"asm {before_asm}->{after_asm} (+{max(added,0)})")
        else:
            print(f"[skip] {target_id}: already granular")

    print(f"\nTotal new split functions: {total_added}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
