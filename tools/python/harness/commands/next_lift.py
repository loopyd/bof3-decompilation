"""Return the first liftable candidate for a target in one call.

Consolidates the per-lane sequence measured in the fan-out runs: run
``rev-query quick-wins``, then screen every row's original bytes for
``div``/``break`` trap opcodes, then take the first survivor. Lanes repeated
that loop for up to 8 rows on every mission.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.analysis.candidates import candidate_address, surviving_rows

_ROOT = Path(__file__).resolve().parents[4]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="bin/harness lift next", description=__doc__)
    parser.add_argument("target", help="target id, e.g. emi/battle/battle/15")
    parser.add_argument(
        "--limit",
        type=int,
        default=12,
        help="initial rows to screen; the pool doubles while nothing survives",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    survivors, _screened = surviving_rows(_ROOT, args.target, args.limit)
    if not survivors:
        if args.json:
            print(json.dumps({"target": args.target, "candidate": None}, indent=2))
        else:
            print(
                f"{args.target}: no candidate survived the claimed + div/break screen"
            )
        return 1
    best = survivors[0]
    address = candidate_address(best)
    if args.json:
        print(json.dumps(best, indent=2, sort_keys=True))
        return 0
    print(
        f"{best['target']}@{address:#010x}"
        f"  insns={best['instruction_count']} size={best['size']}"
        f"  trap_screened={best.get('trap_screened')}"
        f"  unresolved_calls={best['unresolved_calls']}"
    )
    print(f"bin/harness lift gate {best['target']}@{address:#010x} {best['target']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
