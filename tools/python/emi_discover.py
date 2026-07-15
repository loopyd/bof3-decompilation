#!/usr/bin/env python3
"""Discover functions in the remaining EMI targets that lack a `type: code` segment.

Most EMI YAMLs only declare `[0x0, bin]` + `[<size>]` — the code payload is
present in out/binaries/emi/<family>/<archive>/<slot>.bin but Splat has no
code segment to analyze. This tool:

  1. Reads the target's `.bin.json` to find the source extracted payload
     (e.g. out/extracted/BIN/BATTLE/BATTLE/3.bin).
  2. Runs `emi-ex list <archive>.EMI` to learn the entry's real `ram_ptr`
     (the authoritative load address) and `code_off` (offset of the text base
     inside the payload, read from the payload header's t_addr field).
  3. Runs spimdisasm on the promoted binary at code_off/vram to detect
     functions.
  4. If the detection looks like real code (>= 2 coherent functions, the first
     not a garbage `T_` blob), rewrites the YAML to declare a `type: code`
     segment whose subsegments are the detected functions (all `asm`).

Usage:
    python3 tools/python/emi_discover.py              # report only
    python3 tools/python/emi_discover.py --apply       # rewrite YAMLs
    python3 tools/python/emi_discover.py --target emi/battle/battle/03
"""

from __future__ import annotations

import argparse
import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent.parent
SPLAT_DIR = ROOT / "config" / "splat"
EMI_EX = ROOT / "third_party" / "emi-ex-v2" / "target" / "release" / "emi-ex"
VENV_PY = ROOT / ".venv" / "bin" / "python"
PY = str(VENV_PY) if VENV_PY.is_file() else sys.executable


def _to_int(x: object) -> int:
    if isinstance(x, int):
        return x
    if isinstance(x, str):
        return int(x, 0)
    return 0


def list_entry(archive: Path, index: int) -> dict | None:
    """Return {offset, ram_ptr, size, file_type, code_off} for archive entry N."""
    res = subprocess.run(
        [str(EMI_EX), "list", str(archive)],
        capture_output=True, text=True, timeout=60, cwd=ROOT,
    )
    if res.returncode != 0:
        return None
    # Parse the table; first data row is the header line we printed, then rows.
    lines = res.stdout.splitlines()
    # Find rows starting with the index number.
    for line in lines:
        cols = line.split()
        if not cols or not cols[0].isdigit():
            continue
        if int(cols[0]) != index:
            continue
        # IDX OFFSET RAM_PTR SIZE TYPE CODE_OFF FIRST4
        offset = int(cols[1], 0)
        ram_ptr = int(cols[2], 0)
        size = int(cols[3], 0)
        file_type = int(cols[4], 0)
        code_off = int(cols[5], 0) if cols[5] != "-" else None
        return {
            "offset": offset, "ram_ptr": ram_ptr, "size": size,
            "file_type": file_type, "code_off": code_off,
        }
    return None


def spimdisasm_functions(binary: Path, start: int, end: int, vram: int) -> list[dict]:
    with tempfile.TemporaryDirectory() as td:
        out_s = Path(td) / "o.s"
        info = Path(td) / "f.csv"
        cmd = [
            PY, "-m", "spimdisasm", "singleFileDisasm",
            str(binary), str(out_s),
            "--start", hex(start), "--end", hex(end), "--vram", hex(vram),
            "--endian", "little", "--function-info", str(info),
        ]
        try:
            subprocess.run(cmd, capture_output=True, text=True, timeout=300, cwd=ROOT)
        except subprocess.TimeoutExpired:
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


def is_real_code(funcs: list[dict]) -> bool:
    if len(funcs) < 2:
        return False
    # Count genuine functions (func_*); a leading T_ blob is a common
    # spimdisasm artifact for data-aligned overlays and is ignored.
    real = [f for f in funcs if f["name"].startswith("func_") and f["size"] >= 4]
    if len(real) < 2:
        return False
    return True


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--target")
    args = ap.parse_args()

    total = 0
    for yp in sorted(SPLAT_DIR.rglob("*.yaml")):
        rel = yp.relative_to(SPLAT_DIR).with_suffix("")
        target_id = str(rel).replace("\\", "/")
        if args.target and target_id != args.target:
            continue

        doc = yaml.safe_load(yp.read_text(encoding="utf-8"))
        # Skip targets that already declare a code segment.
        has_code = any(
            isinstance(s, dict) and s.get("type") == "code"
            for s in doc.get("segments", [])
        )
        if has_code:
            continue

        binary_rel = doc.get("options", {}).get("target_path")
        if not binary_rel:
            continue
        binary = ROOT / binary_rel
        if not binary.is_file():
            continue

        # Find the source payload + archive from the .bin.json.
        json_path = binary.with_suffix(".bin.json")
        if not json_path.is_file():
            print(f"[skip] {target_id}: no .bin.json (cannot locate EMI entry)")
            continue
        meta = json.loads(json_path.read_text(encoding="utf-8"))
        source = Path(meta.get("source", ""))
        # source like .../BIN/BATTLE/BATTLE/3.bin -> archive .../BIN/BATTLE/BATTLE.EMI, idx 3
        # The .EMI archive sits one level up from the slot folder, named after it.
        archive = source.parent.parent / (source.parent.name + ".EMI")
        try:
            index = int(source.stem)
        except ValueError:
            print(f"[skip] {target_id}: cannot parse entry index from {source}")
            continue
        if not archive.is_file():
            print(f"[skip] {target_id}: archive missing {archive}")
            continue

        entry = list_entry(archive, index)
        if entry is None:
            print(f"[skip] {target_id}: emi-ex list failed for entry {index}")
            continue

        ram_ptr = entry["ram_ptr"]
        size = entry["size"]
        # Type-0 payloads are loaded and executed from offset 0; the header
        # word at 0x18 is the first instruction, not a vram pointer. Try the
        # TOC-derived header offset first, then fall back to offset 0.
        candidates = [c for c in (entry["code_off"], 0) if c is not None]
        best = None
        for code_off in candidates:
            vram = ram_ptr + code_off
            funcs = spimdisasm_functions(binary, code_off, size, vram)
            if is_real_code(funcs):
                best = (code_off, vram, funcs)
                break
        if best is None:
            print(f"[skip] {target_id}: entry {index} not real code "
                  f"(tried offsets {candidates})")
            continue

        code_off, vram, funcs = best

        # Absolute file offsets for each detected function.
        subs = [[code_off + (f["vram"] - vram), "asm", f["name"]] for f in funcs]
        code_end = max(s[0] + s[2] if False else s[0] + f["size"]
                       for s, f in zip(subs, funcs))
        new_seg = {
            "name": "main",
            "type": "code",
            "start": code_off,
            "vram": vram,
            "subsegments": subs,
        }
        if args.apply:
            doc = dict(doc)
            trailing = [[code_end, "bin", "data"], [size]]
            # When code starts at file offset 0 there is no leading bin region;
            # otherwise keep a bin segment marking the raw payload header.
            if code_off == 0:
                doc["segments"] = [new_seg, *trailing]
            else:
                doc["segments"] = [[0x0, "bin", "header"], new_seg, *trailing]
            yp.write_text(yaml.safe_dump(doc, sort_keys=False, default_flow_style=False), encoding="utf-8")
        total += 1
        print(f"[{'apply' if args.apply else 'report'}] {target_id}: "
              f"entry {index} ram_ptr={ram_ptr:#x} code_off={code_off:#x} "
              f"vram={vram:#x} funcs={len(funcs)}")

    print(f"\nTargets with discovered code segments: {total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
