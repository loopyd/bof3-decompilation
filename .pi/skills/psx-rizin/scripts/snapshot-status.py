#!/usr/bin/env python3
"""Report target-qualified Rizin snapshot and reverse-index readiness without mutation."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tomllib
from typing import Any


ROOT = Path(__file__).resolve().parents[4]


def command(root: Path, *args: str) -> dict[str, Any]:
    completed = subprocess.run(args, cwd=root, text=True, capture_output=True)
    result: dict[str, Any] = {"command": list(args), "exit_code": completed.returncode}
    stdout = completed.stdout.strip()
    if stdout:
        try:
            result["json"] = json.loads(stdout)
        except json.JSONDecodeError:
            result["stdout"] = stdout[-2000:]
    if completed.stderr.strip():
        result["stderr"] = completed.stderr.strip()[-2000:]
    return result


def manifests(root: Path) -> dict[str, tuple[Path, dict[str, Any]]]:
    result: dict[str, tuple[Path, dict[str, Any]]] = {}
    for path in sorted((root / "config" / "targets").glob("**/target.toml")):
        data = tomllib.loads(path.read_text())
        result[data["id"]] = (path, data)
    return result


def target_record(root: Path, target: str, manifest_path: Path, manifest: dict[str, Any]) -> dict[str, Any]:
    binary = root / manifest["binary"]
    return {
        "target": target,
        "manifest": manifest_path.relative_to(root).as_posix(),
        "kind": manifest["kind"],
        "disc_id": manifest["disc_id"],
        "load_address": f"0x{int(manifest['load_address']):08X}",
        "binary": {
            "path": binary.relative_to(root).as_posix(),
            "exists": binary.is_file(),
            **({"size": binary.stat().st_size, "sha256": hashlib.sha256(binary.read_bytes()).hexdigest()} if binary.is_file() else {}),
        },
        "snapshot": command(root, "bin/rz-project", "status", target, "--json"),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target", nargs="?", help="one target; omit to sweep every configured target")
    parser.add_argument("--root", type=Path, default=ROOT)
    args = parser.parse_args()
    root = args.root.resolve()
    known = manifests(root)
    if args.target and args.target not in known:
        parser.error(f"unknown target: {args.target}")
    selected = [args.target] if args.target else sorted(known)
    records = [target_record(root, target, *known[target]) for target in selected]
    fresh = sum(bool(record["snapshot"].get("json", {}).get("fresh")) for record in records)
    report = {
        "schema": "bof3.skill-rizin-snapshot-status/v1",
        "targets": records,
        "summary": {"total": len(records), "fresh": fresh, "stale_or_unavailable": len(records) - fresh},
        "reverse_index": command(root, "bin/rev-query", "status", "--json"),
        "next_action": (
            "run bin/rz-project analyze TARGET for each stale target, then bin/index"
            if fresh != len(records)
            else "reverse-index queries are ready when reverse_index exit_code is 0"
        ),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
