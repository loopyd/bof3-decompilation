#!/usr/bin/env python3
"""Audit all BGM tracks for feature coverage: reverb, modulation, noise readiness."""

import json
import os
import subprocess
import sys
from pathlib import Path


def find_bgm_dir(root: Path) -> Path | None:
    for candidate in [
        root / "out" / "extracted" / "BIN" / "BGM",
        root.parent / "out" / "extracted" / "BIN" / "BGM",
        root.parent.parent / "out" / "extracted" / "BIN" / "BGM",
    ]:
        if candidate.is_dir():
            return candidate
    return None


def run_audit(tool: Path, track: Path) -> dict | None:
    try:
        result = subprocess.run(
            [str(tool), "bgm-audit", str(track)],
            capture_output=True,
            text=True,
            timeout=30,
        )
        line = result.stdout.strip()
        if not line or "=" not in line:
            return None
        parts = line.split(": ", 1)
        if len(parts) < 2:
            return None
        data = {}
        for token in parts[1].split():
            if "=" in token:
                k, v = token.split("=", 1)
                try:
                    data[k] = int(v)
                except ValueError:
                    data[k] = v
        return data
    except Exception:
        return None


def main():
    root = Path(__file__).resolve().parent.parent.parent.parent
    tool = root / "tools" / "c" / "psx-audio" / "build" / "bof3-audio"
    if not tool.exists():
        tool = root / "bin" / "psx-audio-bin"
    if not tool.exists():
        print(f"error: tool not found, build first", file=sys.stderr)
        return 1

    bgm_dir = find_bgm_dir(root)
    if not bgm_dir:
        print("error: out/extracted/BIN/BGM not found", file=sys.stderr)
        return 1

    tracks = sorted(bgm_dir.glob("*.EMI"))
    if not tracks:
        print("error: no .emi files found", file=sys.stderr)
        return 1

    total = len(tracks)
    with_reverb = 0
    with_modulation = 0
    total_reverb_tones = 0
    total_modulation_tones = 0
    total_tones = 0
    results = []

    print(f"\n  BGM Feature Audit ({total} tracks)\n")
    print(f"  {'Track':<16} {'Tones':>6} {'RevTones':>9} {'ModTones':>9} {'Rev%':>6} {'Mod%':>6}")
    print(f"  {'─' * 16} {'─' * 6} {'─' * 9} {'─' * 9} {'─' * 6} {'─' * 6}")

    for track in tracks:
        name = track.stem
        data = run_audit(tool, track)
        if not data:
            continue

        tones = data.get("tones", 0)
        rev = data.get("reverb-tones", 0)
        mod = data.get("modulation-tones", 0)
        total_tones += tones
        total_reverb_tones += rev
        total_modulation_tones += mod
        if rev > 0:
            with_reverb += 1
        if mod > 0:
            with_modulation += 1

        rev_pct = f"{100 * rev / tones:.0f}%" if tones > 0 else "—"
        mod_pct = f"{100 * mod / tones:.0f}%" if tones > 0 else "—"

        print(f"  {name:<16} {tones:>6} {rev:>9} {mod:>9} {rev_pct:>6} {mod_pct:>6}")
        results.append(data)

    print(f"  {'─' * 16} {'─' * 6} {'─' * 9} {'─' * 9} {'─' * 6} {'─' * 6}")
    print(f"  {'TOTAL':<16} {total_tones:>6} {total_reverb_tones:>9} {total_modulation_tones:>9}")
    print()
    print(f"  Tracks with reverb tones:    {with_reverb}/{total}")
    print(f"  Tracks with modulation tones: {with_modulation}/{total}")
    print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
