#!/usr/bin/env python3
"""Self-check checkpoint no-progress host-gate semantics."""

import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT / ".pi/skills/bof3-lift-loop/scripts/attempt-checkpoint.py"
SELECTOR = "emi/world00/area030/04@0x801DAE3C"
LANE = "checkpoint-self-check"


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=ROOT, text=True, capture_output=True)


def main() -> int:
    lane_dir = ROOT / "out/lift-loop/checkpoints" / LANE
    first = run("python3", str(SCRIPT), "capture", "--lane", LANE, "--selector", SELECTOR, "--attempt", "1", "--match", "83.33")
    assert first.returncode == 0, first.stderr
    second = run("python3", str(SCRIPT), "capture", "--lane", LANE, "--selector", SELECTOR, "--attempt", "2", "--match", "83.33", "--require-improvement", "--soft-no-improvement")
    outcome = json.loads(second.stdout)
    assert second.returncode == 0 and outcome["accepted"] is False and outcome["exit_code"] == 1
    third = run("python3", str(SCRIPT), "capture", "--lane", LANE, "--selector", SELECTOR, "--attempt", "3", "--match", "0", "--require-improvement", "--soft-no-improvement")
    assert third.returncode == 2, third.stdout
    subprocess.run(("rm", "-rf", str(lane_dir)), check=True)
    print("attempt checkpoint self-check: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
