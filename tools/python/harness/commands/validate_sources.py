"""Validate every tracked lift has its required evidence metadata and a diff."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re

from ..domain import load_target_manifests
from ..io import repo_layout
from ..match.asm_diff import AsmDiffRequest, run_asm_diff_one
from ._common import run_main


_SOURCE = re.compile(r"@source 0x[0-9A-F]{8}\b")
_BEHAVIOR = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")
_UNDEFINED = re.compile(r"undefined reference to `([^']+)'" )


def _failure_detail(error: Exception) -> str:
    text = str(error)
    symbols = sorted(set(_UNDEFINED.findall(text)))
    if symbols:
        return f"unbound symbols: {', '.join(symbols)}"
    return text.splitlines()[0]


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    manifests = load_target_manifests(root)
    failures: list[str] = []
    report: list[dict[str, str]] = []
    exact = 0
    partial = 0
    for manifest in manifests.values():
        source_dir = root / manifest.source_dir
        for source in sorted(source_dir.glob("func_*.c")):
            text = source.read_text(encoding="utf-8")
            if _SOURCE.search(text) is None or _BEHAVIOR.search(text) is None:
                failures.append(f"metadata: {source.relative_to(root)}")
                report.append({"source": source.relative_to(root).as_posix(), "status": "invalid", "reason": "missing required metadata"})
                continue
            try:
                address = int(source.stem.removeprefix("func_"), 16)
                result = run_asm_diff_one(
                    AsmDiffRequest(
                        source_path=source,
                        address=address,
                        binary_path=root / manifest.binary,
                        load_address=manifest.load_address,
                        output_root=root / "out" / "matching",
                    )
                )
            except (FileNotFoundError, RuntimeError, ValueError) as exc:
                detail = _failure_detail(exc)
                failures.append(f"diff: {source.relative_to(root)}: {detail}")
                report.append({"source": source.relative_to(root).as_posix(), "status": "invalid", "reason": detail})
                continue
            if result["byte_match"]:
                exact += 1
                status = "exact"
            else:
                partial += 1
                status = "partial"
            report.append({"source": source.relative_to(root).as_posix(), "status": status, "reason": ""})
    if args.out is not None:
        output = args.out if args.out.is_absolute() else root / args.out
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for failure in failures:
        print(failure)
    print(f"lifts: exact={exact} partial={partial} invalid={len(failures)}")
    return 2 if failures else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="validate-sources")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("-o", "--out", type=Path, help="write a JSON audit report")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
