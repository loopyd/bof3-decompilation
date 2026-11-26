"""List current naming facts, check their consistency or rename one spelling."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main
from .inspection import KINDS, check_batch, check_report, list_report
from .rename import (
    apply_group,
    apply_report,
    apply_rows,
    plan_batch,
    plan_group,
    plan_report,
)


def read_plan(argument: str) -> list[dict]:
    """Read a batch plan: JSON rows, or one `TARGET\tOLD\tNEW` line per row."""
    path = Path(argument)
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as error:
        raise SystemExit(f"{path}: {error.strerror}")
    if path.suffix == ".json":
        payload = json.loads(text)
        rows = payload["rows"] if isinstance(payload, dict) else payload
        if not isinstance(rows, list):
            raise SystemExit(f"{path}: expected a list of rows")
        return rows
    rows = []
    for number, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) != 3:
            raise SystemExit(f"{path}:{number}: expected TARGET<TAB>OLD<TAB>NEW")
        rows.append({"target": parts[0], "old": parts[1], "new": parts[2]})
    return rows


def check_in_parallel(root, targets: list[str], jobs: int) -> list[dict]:
    """Check independent targets in worker processes, each with one catalog."""
    from concurrent.futures import ProcessPoolExecutor

    if jobs <= 1 or len(targets) < 2:
        return check_batch(root, targets)
    chunks = [targets[index::jobs] for index in range(jobs)]
    chunks = [chunk for chunk in chunks if chunk]
    with ProcessPoolExecutor(max_workers=len(chunks)) as pool:
        results = list(pool.map(_check_chunk, [(str(root), chunk) for chunk in chunks]))
    by_target = {}
    for chunk_reports in results:
        for report in chunk_reports:
            by_target[report["scope"]["target"]] = report
    ordered = [by_target[target] for target in targets if target in by_target]
    # Concurrent builds can report a link failure that a quiet tree does not have, so
    # every target that reported one is confirmed serially before it is believed.
    suspect = [
        report["scope"]["target"]
        for report in ordered
        if any(finding["code"] == "native" for finding in report["findings"])
    ]
    if suspect:
        for report in check_batch(root, suspect):
            by_target[report["scope"]["target"]] = report
        ordered = [by_target[target] for target in targets if target in by_target]
    return ordered


def _check_chunk(job) -> list[dict]:
    from pathlib import Path

    root, chunk = job
    return check_batch(Path(root), chunk)


def read_targets(path) -> list[str]:
    from pathlib import Path

    return [line.strip() for line in Path(path).read_text(encoding="utf-8").splitlines() if line.strip()]


def all_targets(root) -> list[str]:
    from harness.domain.manifests import load_target_manifests

    return sorted(load_target_manifests(root))


def run(args: argparse.Namespace) -> int:
    if args.action == "batch":
        rows = read_plan(args.plan)
        result = (
            apply_rows(args.root, rows, jobs=args.jobs)
            if args.apply
            else plan_batch(args.root, rows)
        )
        if args.json:
            print(json.dumps(result, indent=2, sort_keys=True))
            return 0 if result["passed"] else 1
        records = result["records"]
        applied = sum(1 for record in records if record["applied"])
        print(
            f"naming batch: rows={len(records)} applied={applied} "
            f"passed={result['passed']} refusals={len(result['refusals'])}"
        )
        tally: dict[str, int] = {}
        for record in records:
            if record["applied"]:
                continue
            codes = ",".join(sorted({row["code"] for row in record["refusals"]})) or "clean"
            tally[codes] = tally.get(codes, 0) + 1
            scope = record["scope"]
            print(
                json.dumps(
                    {
                        "target": scope["target"],
                        "old": scope["old"],
                        "new": scope["new"],
                        "codes": codes.split(",") if codes != "clean" else [],
                    },
                    sort_keys=True,
                )
            )
        if tally:
            print("refusals by code: " + json.dumps(tally, sort_keys=True))
        return 0 if result["passed"] else 1
    if args.action == "rename":
        selections = [(args.target, args.old)]
        for pair in args.with_target:
            target, _, old = pair.partition("=")
            if not target or not old:
                raise SystemExit(f"--with expects TARGET=OLD, got {pair!r}")
            selections.append((target, old))
        if args.with_target:
            result = (
                apply_group(args.root, args.new, selections)
                if args.apply
                else plan_group(args.root, args.new, selections)
            )
        else:
            result = (
                apply_report(args.root, args.target, args.old, args.new)
                if args.apply
                else plan_report(args.root, args.target, args.old, args.new)
            )
    elif args.action == "check" and (args.all or args.targets_file):
        targets = all_targets(args.root) if args.all else read_targets(args.targets_file)
        reports = check_in_parallel(args.root, targets, args.jobs)
        if args.json:
            print(json.dumps(reports, indent=2, sort_keys=True))
            return 0 if all(report["passed"] for report in reports) else 1
        failed = 0
        for report in reports:
            codes: dict[str, int] = {}
            for finding in report["findings"]:
                codes[finding["code"]] = codes.get(finding["code"], 0) + 1
            if not report["passed"]:
                failed += 1
            if codes or not args.summary:
                print(json.dumps({"target": report["scope"]["target"],
                                  "records": len(report["records"]),
                                  "passed": report["passed"],
                                  "findings": codes}, sort_keys=True))
        print(f"targets={len(reports)} failed={failed}")
        return 0 if failed == 0 else 1
    elif args.action == "check":
        result = check_report(args.root, args.target, tuple(args.selectors))
    else:
        result = list_report(args.root, args.target, kind=args.kind, limit=args.limit)
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0 if result["passed"] else 1
    if args.action == "rename" and "members" in result:
        print(f"naming rename-group: new={result['new']} applied={result['applied']} passed={result['passed']}")
        for member in result["members"]:
            print(json.dumps({"target": member["scope"]["target"], "old": member["scope"]["old"],
                              "address": member["address"], "passed": member["passed"],
                              "edits": [row["file"] for row in member["edits"]],
                              "renames": member["renames"], "refusals": member["refusals"],
                              "proof": member["proof"]}, sort_keys=True))
        for row in result["refusals"]:
            print(json.dumps(row, sort_keys=True))
        return 0 if result["passed"] else 1
    if args.action == "rename":
        print(
            f"naming rename: {json.dumps(result['scope'], sort_keys=True)} "
            f"applied={result['applied']} passed={result['passed']}"
        )
        for row in result["edits"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["renames"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["proof"]:
            print(json.dumps(row, sort_keys=True))
        for row in result["refusals"]:
            print(json.dumps(row, sort_keys=True))
        return 0 if result["passed"] else 1
    print(f"naming {result['action']}: {json.dumps(result['scope'], sort_keys=True)}")
    for row in result["records"]:
        print(json.dumps(row, sort_keys=True))
    for declaration in result.get("declarations", ()):
        print(json.dumps(declaration, sort_keys=True))
    if "consumers" in result:
        print("consumers=" + json.dumps(result["consumers"]))
    for finding in result["findings"]:
        print(json.dumps(finding, sort_keys=True))
    print(
        f"records={result['total']} truncated={result['truncated']} "
        f"incomplete={result['incomplete']} passed={result['passed']}"
    )
    return 0 if result["passed"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness naming")
    add_example_argument(parser, "bin/harness naming list exe/logo --limit 10")
    actions = parser.add_subparsers(dest="action", required=True)
    batch = actions.add_parser("batch")
    add_root_argument(batch)
    batch.add_argument("--plan", required=True, metavar="FILE")
    batch.add_argument("--jobs", type=int, default=4)
    batch.add_argument("--json", action="store_true")
    batch.add_argument("--apply", action="store_true")
    batch.set_defaults(handler=run)
    for name in ("list", "check", "rename"):
        action = actions.add_parser(name)
        add_root_argument(action)
        action.add_argument("target", metavar="TARGET", nargs="?" if name == "check" else None)
        action.add_argument("--json", action="store_true")
        if name == "list":
            action.add_argument("--kind", choices=KINDS, default="debt")
            action.add_argument("--limit", type=int)
        elif name == "check":
            action.add_argument("selectors", metavar="SELECTOR", nargs="*")
            action.add_argument("--all", action="store_true",
                                help="check every target against one manifest generation")
            action.add_argument("--targets-file", metavar="FILE",
                                help="check the targets listed one per line")
            action.add_argument("--summary", action="store_true",
                                help="print only the targets that have findings")
            action.add_argument("--jobs", type=int, default=1,
                                help="check targets in this many worker processes")
        else:
            action.add_argument("old", metavar="OLD")
            action.add_argument("new", metavar="NEW")
            action.add_argument(
                "--with",
                dest="with_target",
                action="append",
                default=[],
                metavar="TARGET=OLD",
                help="another target that must carry the same spelling, with its own old name",
            )
            action.add_argument("--apply", action="store_true")
        action.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
