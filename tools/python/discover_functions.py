#!/usr/bin/env python3
"""Find proper functions in BOF3 binaries and split coarse Splat subsegments.

spimdisasm (the canonical PSX disassembler Splat wraps) auto-detects function
boundaries by following the call graph from each segment's entry point. Many
Splat subsegments are coarse: a single `asm` subsegment of several kilobytes
actually contains many proper functions. This tool splits `asm` subsegments
into the functions spimdisasm detects, while leaving already-lifted `c`
subsegments untouched (splitting a 100%-matched function would break it).

Usage:
    python3 tools/python/discover_functions.py            # report only
    python3 tools/python/discover_functions.py --apply    # rewrite YAMLs

Rules (per code segment):
  * A detected function whose start equals a declared subsegment start is kept
    as-is (already mapped).
  * A detected function starting inside a declared `c` extent is skipped (do
    not touch lifted functions).
  * A detected function starting inside a declared `asm` extent is added as a
    new `asm` subsegment, splitting the parent.
  * `bin`/`data`/rodata subsegments are preserved and never receive functions.

Output subsegments are emitted in ascending file-offset order; the parent
`asm` subsegment is replaced by its proper-function pieces.
"""

from __future__ import annotations

import argparse
import csv
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
    top_starts: list[int] = []
    for s in segments:
        if isinstance(s, dict):
            top_starts.append(_to_int(s.get("start", 0)))
        elif isinstance(s, list) and s:
            top_starts.append(_to_int(s[0]))
    top_starts = sorted(set(top_starts))
    code_segs: list[CodeSegment] = []
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
        code_segs.append(CodeSegment(s.get("name", "code"), start, vram, end))
    return code_segs


def load_declared(yaml_path: Path) -> list[DeclaredSub]:
    doc = yaml.safe_load(yaml_path.read_text(encoding="utf-8"))
    declared: list[DeclaredSub] = []
    for s in doc.get("segments", []):
        if not isinstance(s, dict):
            continue
        seg_start = _to_int(s.get("start", 0))
        seg_vram = _to_int(s.get("vram", 0))
        for sub in s.get("subsegments", []):
            off = _to_int(sub[0])
            kind = sub[1]
            name = sub[2] if len(sub) > 2 else ""
            vram = seg_vram + (off - seg_start)
            declared.append(DeclaredSub(off, vram, kind, name))
    return declared


def detect_functions(binary: Path, seg: CodeSegment) -> list[dict]:
    with tempfile.TemporaryDirectory() as td:
        out_s = Path(td) / "out.s"
        info = Path(td) / "funcinfo.csv"
        cmd = [
            sys.executable, "-m", "spimdisasm", "singleFileDisasm",
            str(binary), str(out_s),
            "--start", hex(seg.start),
            "--end", hex(seg.end),
            "--vram", hex(seg.vram),
            "--endian", "little",
            "--function-info", str(info),
        ]
        try:
            subprocess.run(cmd, capture_output=True, text=True, timeout=300, cwd=ROOT)
        except subprocess.TimeoutExpired:
            print(f"  WARN: spimdisasm timed out for {binary} seg {seg.name}", file=sys.stderr)
            return []
        if not info.is_file():
            return []
        funcs: list[dict] = []
        with info.open(encoding="utf-8") as fh:
            for row in csv.DictReader(fh):
                funcs.append({
                    "vram": int(row["address"], 0),
                    "size": int(row["length"], 0),
                    "name": row["name"],
                })
        return funcs


def resolve_targets(yaml_path: Path) -> list[dict]:
    """Return ordered list of (offset, kind, name) subsegments per code segment.

    Splits coarse `asm` subsegments into the proper functions spimdisasm
    detects. `c` subsegments and bin/data/rodata are preserved verbatim.
    """
    doc = yaml.safe_load(yaml_path.read_text(encoding="utf-8"))
    binary_rel = doc.get("options", {}).get("target_path")
    if not binary_rel:
        return []
    binary = ROOT / binary_rel
    if not binary.is_file():
        return []

    code_segs = load_code_segments(yaml_path)
    for seg in code_segs:
        if seg.end <= seg.start:
            seg.end = len(binary.read_bytes())

    new_segments: list[dict] = []
    changed = False
    for s in doc.get("segments", []):
        if not isinstance(s, dict) or s.get("type") != "code":
            new_segments.append(s)
            continue
        seg_start = _to_int(s["start"])
        seg_vram = _to_int(s["vram"])
        seg_end = next((c.end for c in code_segs if c.start == seg_start), seg_start)
        # bounding subsegment for this code segment: segment end in file offsets
        seg_file_end = seg_start + (seg_end - seg_vram) - (seg_vram - seg_vram)
        # seg_end is a vram; convert to file offset using this segment's mapping
        seg_file_end = seg_start + (seg_end - seg_vram)

        seg = next(c for c in code_segs if c.start == seg_start)
        detected = detect_functions(binary, seg)

        # Build proper boundaries within this segment's file range.
        # Keep declared subs that are not coarse asm.
        declared_subs = load_declared(yaml_path)
        in_seg_decl = sorted(
            [d for d in declared_subs if seg.vram <= d.vram < seg.vram + (seg.end - seg.start)],
            key=lambda d: d.offset,
        )

        # Determine extents of declared c subsegments (do-not-touch zones).
        c_extents: list[tuple[int, int]] = []
        asm_extents: list[tuple[int, int]] = []
        for i, d in enumerate(in_seg_decl):
            nxt = in_seg_decl[i + 1].vram if i + 1 < len(in_seg_decl) else seg.vram + (seg.end - seg.start)
            if d.kind == "c":
                c_extents.append((d.vram, nxt))
            elif d.kind == "asm":
                asm_extents.append((d.vram, nxt))

        # Collect boundary vrams: declared starts + detected starts in asm extents.
        boundaries: list[tuple[int, str, str]] = []  # (vram, kind, name)
        # declared starts
        for d in in_seg_decl:
            boundaries.append((d.vram, d.kind, d.name))
        # detected starts inside asm extents (split asm)
        for fn in detected:
            addr = fn["vram"]
            if any(a <= addr < b for a, b in asm_extents) and addr not in {b[0] for b in boundaries}:
                boundaries.append((addr, "asm", f"func_{addr:08x}"))
        # sort, drop anything inside c extents (already kept via declared start)
        boundaries = sorted({b for b in boundaries}, key=lambda b: b[0])
        boundaries = [b for b in boundaries if not any(c <= b[0] < e for c, e in c_extents) or b[1] == "c"]

        # Build subsegments in file-offset order.
        new_subs: list[list] = []
        for i, (vram, kind, name) in enumerate(boundaries):
            file_off = seg_start + (vram - seg_vram)
            new_subs.append([file_off, kind, name])
        if new_subs != [[d.offset, d.kind, d.name] for d in in_seg_decl]:
            changed = True
        s = dict(s)
        s["subsegments"] = new_subs
        new_segments.append(s)

    return new_segments if changed else doc.get("segments", [])


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true", help="rewrite YAMLs with split functions")
    ap.add_argument("--target", help="limit to one target id")
    args = ap.parse_args()

    yaml_files = sorted(SPLAT_DIR.rglob("*.yaml"))
    total_added = 0
    for yp in yaml_files:
        rel = yp.relative_to(SPLAT_DIR).with_suffix("")
        target_id = str(rel).replace("\\", "/")
        if args.target and target_id != args.target:
            continue
        before = load_declared(yp)
        before_asm = sum(1 for d in before if d.kind == "asm")
        before_c = sum(1 for d in before if d.kind == "c")

        doc = yaml.safe_load(yp.read_text(encoding="utf-8"))
        binary_rel = doc.get("options", {}).get("target_path")
        if not binary_rel:
            continue
        binary = ROOT / binary_rel
        if not binary.is_file():
            continue

        code_segs = load_code_segments(yp)
        for seg in code_segs:
            if seg.end <= seg.start:
                seg.end = len(binary.read_bytes())

        new_segments: list = []
        changed = False
        for s in doc.get("segments", []):
            if not isinstance(s, dict) or s.get("type") != "code":
                new_segments.append(s)
                continue
            seg_start = _to_int(s["start"])
            seg_vram = _to_int(s["vram"])
            seg = next(c for c in code_segs if c.start == seg_start)
            detected = detect_functions(binary, seg)

            declared_subs = load_declared(yp)
            in_seg_decl = sorted(
                [d for d in declared_subs if seg.vram <= d.vram < seg.vram + (seg.end - seg.start)],
                key=lambda d: d.offset,
            )
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
            boundaries = [b for b in boundaries if (b[1] == "c") or not any(c <= b[0] < e for c, e in c_extents)]

            new_subs: list[list] = []
            for vram, kind, name in boundaries:
                file_off = seg_start + (vram - seg_vram)
                new_subs.append([file_off, kind, name])
            old_subs = [[d.offset, d.kind, d.name] for d in in_seg_decl]
            if new_subs != old_subs:
                changed = True
            s = dict(s)
            s["subsegments"] = new_subs
            new_segments.append(s)

        added = 0
        if changed:
            added = sum(1 for ns in new_segments if isinstance(ns, dict)
                        for sub in ns.get("subsegments", [])
                        if sub[1] == "asm" and sub[2].startswith("func_")) - before_asm
            if args.apply:
                doc = dict(doc)
                doc["segments"] = new_segments
                yp.write_text(yaml.safe_dump(doc, sort_keys=False, default_flow_style=False), encoding="utf-8")
            total_added += max(added, 0)
            print(f"[{'apply' if args.apply else 'report'}] {target_id}: "
                  f"asm {before_asm}->{sum(1 for ns in new_segments if isinstance(ns,dict) for sub in ns.get('subsegments',[]) if sub[1]=='asm')}, "
                  f"c={before_c}, +{max(added,0)} split functions")
        else:
            print(f"[skip] {target_id}: already granular")

    print(f"\nTotal new split functions: {total_added}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
