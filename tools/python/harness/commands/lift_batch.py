"""Select a batch of lift lanes across targets with the measured screens.

``lift next`` answers one target. Campaign runs need ten disjoint targets per
wave, and two failure classes are cheap to reject before a lane starts: an
in-payload code-pointer run at the candidate address (a data head, not a
function) and an address that the SDK map already owns. Both were found by
hand in earlier waves, so they are mechanical screens here.

Contract refs and revisions come out as ``{key, target, refs}`` lanes ready for
``.pi/workflows/bof3-lift-lanes.js``.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

from harness.commands.next_lift import (
    _ROOT,
    _address,
    _load_address,
    _surviving_rows,
)
from harness.common.commands import command_at

_HEAD_WORDS = 7
_HEAD_POINTERS = 3
_SDK_MAPS = ("config/sdk/psyq-slus.txt", "config/sdk/psyq-logo.txt")


def _targets(root: Path, limit: int) -> list[str]:
    """Return distinct unlifted targets in quick-wins rank order."""
    result = subprocess.run(
        command_at(
            root,
            "rev-query",
            "quick-wins",
            "--unlifted",
            "--detail",
            "minimal",
            "--limit",
            str(limit),
            "--json",
        ),
        cwd=root,
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        sys.stderr.write(result.stdout + result.stderr)
        raise SystemExit(result.returncode)
    ordered: list[str] = []
    for row in json.loads(result.stdout):
        target = row["id"].split("@")[0]
        if target not in ordered:
            ordered.append(target)
    return ordered


def _sdk_addresses(root: Path) -> set[int]:
    addresses: set[int] = set()
    for name in _SDK_MAPS:
        path = root / name
        if not path.is_file():
            continue
        for line in path.read_text(encoding="utf-8").splitlines():
            if "=" not in line:
                continue
            _, _, value = line.partition("=")
            value = value.strip().rstrip(";")
            if value.startswith("0x"):
                try:
                    addresses.add(int(value, 16))
                except ValueError:
                    continue
    return addresses


def _is_data_head(root: Path, target: str, address: int) -> bool:
    """True when the candidate opens with a run of in-payload pointers."""
    load = _load_address(root, target)
    binary = root / "out" / "binaries" / f"{target}.bin"
    if load is None or not binary.is_file():
        return False
    data = binary.read_bytes()
    offset = address - load
    if offset < 0 or offset + 4 * _HEAD_WORDS > len(data):
        return False
    words = [
        int.from_bytes(data[offset + 4 * index : offset + 4 * index + 4], "little")
        for index in range(_HEAD_WORDS)
    ]
    return sum(1 for word in words if load <= word < load + len(data)) >= _HEAD_POINTERS


def _lane(target: str, address: int) -> dict:
    return {
        "key": target.replace("/", "_"),
        "target": target,
        "refs": [f"{target}@{address:#010x}"],
    }


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

    root = _ROOT
    excluded = {value.lower() for value in args.exclude}
    sdk = _sdk_addresses(root)
    lanes: list[dict] = []
    skipped: list[dict] = []
    for target in _targets(root, args.pool):
        if len(lanes) >= args.limit:
            break
        survivors, _screened = _surviving_rows(root, target, 12)
        if not survivors:
            continue
        address = _address(survivors[0])
        selector = f"{target}@{address:#010x}"
        if selector.lower() in excluded:
            skipped.append({"selector": selector, "reason": "excluded"})
            continue
        if address in sdk:
            skipped.append({"selector": selector, "reason": "sdk_address"})
            continue
        if _is_data_head(root, target, address):
            skipped.append({"selector": selector, "reason": "data_head"})
            continue
        lanes.append(_lane(target, address))

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
