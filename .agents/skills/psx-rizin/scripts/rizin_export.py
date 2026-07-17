#!/usr/bin/env python3
"""Export a reproducible JSON/text inventory from a raw PS1 binary in Rizin."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from rizin_common import RizinError, parse_json_output, run_rizin


def parse_int(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--base", type=parse_int, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--rizin", default="rizin")
    parser.add_argument("--no-analysis", action="store_true")
    args = parser.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    commands = {
        "info": "ij",
        "functions": "aflj",
        "strings": "izj",
        "xrefs": "axlj",
    }
    manifest: dict[str, object] = {
        "binary": str(args.binary),
        "base": f"0x{args.base:08x}",
        "analysis": not args.no_analysis,
        "exports": {},
    }

    try:
        for name, command in commands.items():
            stdout, stderr = run_rizin(
                args.binary,
                args.base,
                command,
                rizin=args.rizin,
                analyze=not args.no_analysis,
            )
            (args.out / f"{name}.raw.txt").write_text(stdout, encoding="utf-8")
            if stderr:
                (args.out / f"{name}.stderr.txt").write_text(stderr, encoding="utf-8")
            value = parse_json_output(stdout)
            destination = args.out / f"{name}.json"
            destination.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
            manifest["exports"][name] = str(destination)
    except (OSError, RizinError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2

    (args.out / "manifest.json").write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    print(args.out / "manifest.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
