"""``bin/harness lift campaign``: delta-based lift campaign bookkeeping.

One call updates dispositions, coverage totals, the partials requeue artifact,
snapshots, the index and the repository gate.  Counts are moved mechanically
(exact/partial +1, lifted +1, unlifted -1 for the owning target) so the parent
never hand-computes per-target tuples.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

from harness.common.cli import add_example_argument, run_main
from harness.io import repo_layout

USAGE = (
    "usage: bin/harness lift campaign add SELECTOR exact|partial "
    "[SELECTOR STATUS ...]\n"
    "       bin/harness lift campaign sync"
)
EXAMPLE = "bin/harness lift campaign sync"
_INDEX_LOG = Path("/tmp/campaign-consolidate-index.log")
_CHECK_LOG = Path("/tmp/campaign-consolidate-check.log")
_STATUS = {
    "exact": ("accepted-exact", "exact"),
    "partial": ("partial-documented", "partial"),
}
_TOTAL_KEYS = ("lifted", "exact", "partial", "invalid", "unlifted", "indexed", "sdk")


def _campaign_bookkeeping(root: Path, args: list[str]) -> int:
    campaign = root / "out" / "reviews" / "lift-campaign"
    dispositions_path = campaign / "dispositions.json"
    coverage_path = campaign / "coverage.json"
    dispositions = (
        json.loads(dispositions_path.read_text()) if dispositions_path.is_file() else {}
    )
    rows = json.loads(coverage_path.read_text())
    by_target = {row["target"]: row for row in rows["targets"]}

    added: list[tuple[str, str]] = []
    if args and args[0] == "add":
        pairs = args[1:]
        if len(pairs) % 2:
            print("lift campaign: add needs SELECTOR STATUS pairs", file=sys.stderr)
            return 1
        for selector, status in zip(pairs[::2], pairs[1::2], strict=False):
            if status not in _STATUS:
                print(f"lift campaign: unknown status {status!r}", file=sys.stderr)
                return 1
            internal, field = _STATUS[status]
            if (
                selector in dispositions
                and dispositions[selector]["disposition"] == internal
            ):
                continue
            target, _, address = selector.partition("@")
            row = by_target.get(target)
            if row is None:
                print(f"lift campaign: unknown target {target!r}", file=sys.stderr)
                return 1
            dispositions[selector] = {
                "disposition": internal,
                "evidence": (
                    f"{field} {address} recorded via bin/harness lift campaign"
                ),
            }
            row[field] += 1
            row["lifted"] += 1
            row["unlifted"] = max(0, row["unlifted"] - 1)
            added.append((selector, field))

    sdk_by_target: dict[str, int] = {}
    unlifted_by_target: dict[str, int] = {}
    sdk_linkage_path = campaign / "sdk_linkage.json"
    if sdk_linkage_path.is_file():
        linkage = json.loads(sdk_linkage_path.read_text()).get("targets", {})
        for target, entry in linkage.items():
            sdk_by_target[target] = entry.get(
                "linked_label_exact", entry.get("linked", 0)
            )
            if "unlifted" in entry:
                unlifted_by_target[target] = entry["unlifted"]
    for row in rows["targets"]:
        target = row["target"]
        row["sdk"] = sdk_by_target.get(target, 0)
        if target in unlifted_by_target:
            row["unlifted"] = unlifted_by_target[target]
        elif row["sdk"]:
            row["unlifted"] = max(
                0, row["indexed"] - row["exact"] - row["partial"] - row["sdk"]
            )
    for key in _TOTAL_KEYS:
        rows["totals"][key] = sum(row.get(key, 0) for row in rows["targets"])
    rows["totals"].pop("unlifted_in_scope", None)
    rows["in_scope_unlifted"] = rows["totals"]["unlifted"]
    rows["accounting"] = {
        "model": "indexed = exact + partial + sdk + unlifted",
        "sdk_linked": rows["totals"]["sdk"],
        "unlifted_in_scope": rows["totals"]["unlifted"],
        "evidence": "sdk_linkage.json (PsyQ library/object/symbol per linked function)",
    }

    dispositions_path.write_text(
        json.dumps(dispositions, indent=2, sort_keys=True) + "\n"
    )
    coverage_path.write_text(json.dumps(rows, indent=2, sort_keys=True) + "\n")

    partials = {
        key: value
        for key, value in dispositions.items()
        if value["disposition"].startswith("partial")
    }
    (campaign / "requeue.json").write_text(
        json.dumps(
            {
                "schema": "bof3.lift-campaign/requeue/v1",
                "partial_count": len(partials),
                "note": (
                    "Every partial keeps @status partial/@match/@residual in source "
                    "and records the smallest missing evidence; the remaining lever "
                    "is an opt-in rung (flag-search / compiler-variants / permute / "
                    "per-object profile) or an allocator/scheduler ceiling the "
                    "operator ruled optional."
                ),
                "partials": partials,
            },
            indent=2,
            sort_keys=True,
        )
        + "\n"
    )

    print(f"added {len(added)} disposition(s)")
    print(
        "TOTALS",
        rows["totals"],
        "| sdk-linked",
        rows["totals"]["sdk"],
        "| in-scope unlifted",
        rows["in_scope_unlifted"],
    )
    return 0


def _changed_targets(args: list[str]) -> list[str]:
    changed: list[str] = []
    if args and args[0] == "add" and len(args) > 2:
        rest = args[1:]
        for index in range(0, len(rest) - 1, 2):
            target = rest[index].partition("@")[0]
            if target not in changed:
                changed.append(target)
    return changed


def _refresh(root: Path, changed: list[str]) -> None:
    harness = str(root / "bin" / "harness")
    for target in changed:
        subprocess.run(
            [harness, "analysis", "rz-project", "analyze", target],
            check=False,
            capture_output=True,
            text=True,
        )
    subprocess.run(
        [harness, "source", "symbols", "baseline", "--write"],
        check=False,
        capture_output=True,
        text=True,
    )
    with _INDEX_LOG.open("w") as log:
        status = subprocess.run(
            [harness, "analysis", "index"],
            check=False,
            stdout=log,
            stderr=subprocess.STDOUT,
            cwd=root,
        ).returncode
    if status == 0:
        print("index OK")
    else:
        print(
            "index rc=1 (stale snapshot from an active lane; rebuild at next boundary)"
        )
    just = shutil.which("just") or str(root / "just")
    with _CHECK_LOG.open("w") as log:
        status = subprocess.run(
            [just, "check"],
            check=False,
            stdout=log,
            stderr=subprocess.STDOUT,
            cwd=root,
        ).returncode
    if status == 0:
        print("just check OK")
    else:
        tail = _CHECK_LOG.read_text().splitlines()[-2:]
        for line in tail:
            print(line)


def _run(args: argparse.Namespace) -> int:
    tokens = [args.action, *args.items]
    root = repo_layout().root
    status = _campaign_bookkeeping(root, tokens)
    if status:
        return status
    _refresh(root, _changed_targets(tokens))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness lift campaign", description=__doc__
    )
    parser.add_argument("action", nargs="?", choices=("add", "sync"), default="sync")
    parser.add_argument("items", nargs="*", help="SELECTOR STATUS pairs for add")
    add_example_argument(parser, EXAMPLE)
    parser.set_defaults(handler=_run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
