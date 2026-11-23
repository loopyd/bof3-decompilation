"""Select a batch of lift lanes across targets with the measured screens.

``lift next`` answers one target. Campaign runs need ten disjoint targets per
wave, and two failure classes are cheap to reject before a lane starts: an
in-payload code-pointer run at the candidate address (a data head, not a
function) and an address that the SDK map already owns.

Lanes come out as ``{key, target, refs}``, ready for
``.pi/workflows/bof3-lift-lanes.js``.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.analysis.candidates import select_lanes

_ROOT = Path(__file__).resolve().parents[4]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="bin/harness lift batch", description=__doc__)
    parser.add_argument(
        "--limit", type=int, default=10, help="lanes to return (default: 10)"
    )
    parser.add_argument(
        "--pool",
        type=int,
        default=400,
        help="unlifted rows to consider before giving up (default: 400)",
    )
    parser.add_argument(
        "--exclude",
        action="append",
        default=[],
        help="selector to skip; repeat for several",
    )
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args(argv)

    lanes, skipped = select_lanes(
        _ROOT,
        limit=args.limit,
        pool=args.pool,
        exclude=set(args.exclude),
    )
    if args.json:
        print(json.dumps({"lanes": lanes, "skipped": skipped}, indent=2))
    else:
        for lane in lanes:
            print(f"{lane['target']}  {lane['refs'][0]}")
        for row in skipped:
            print(f"skipped {row['selector']}: {row['reason']}")
        print(f"{len(lanes)} lane(s)")
    return 0 if lanes else 1


if __name__ == "__main__":
    raise SystemExit(main())
