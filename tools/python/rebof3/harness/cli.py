from __future__ import annotations

import argparse
from pathlib import Path

from ..commands._common import run_main
from .commands import (
    run_candidates,
    run_claim,
    run_finish,
    run_ghidra_analyze,
    run_ghidra_coverage,
    run_ghidra_export,
    run_ghidra_import_project,
    run_lift,
    run_release,
    run_refresh,
    run_report_function,
    run_report_module,
    run_report_summary,
    run_status,
    run_verify_function,
    run_verify_module,
)


def configure_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--config", type=Path)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    subparsers = parser.add_subparsers(required=True)

    refresh = subparsers.add_parser(
        "refresh",
        help="refresh harness state from catalog, inventory, and source files",
    )
    refresh.add_argument(
        "--allow-missing-catalog",
        action="store_true",
        help="seed available source targets even when extracted catalog data is absent",
    )
    refresh.set_defaults(handler=run_refresh)

    status = subparsers.add_parser("status", help="project dashboard")
    status.add_argument(
        "--module",
        help="filter by EMI/module, e.g. GAME#0",
    )
    status.set_defaults(handler=run_status)

    candidates = subparsers.add_parser(
        "candidates",
        help="list next function targets for a module",
    )
    candidates.add_argument("--module", required=True)
    candidates.add_argument("--min-size", type=int, default=0)
    candidates.add_argument(
        "--source",
        choices=("missing", "existing", "any"),
        default="missing",
    )
    candidates.add_argument("--limit", type=int, default=20)
    candidates.add_argument(
        "--priority",
        action="store_true",
        help="sort by queue priority instead of largest function size",
    )
    candidates.set_defaults(handler=run_candidates)

    claim = subparsers.add_parser(
        "claim",
        help="claim next unclaimed function in a module",
    )
    claim.add_argument("target_id", nargs="?")
    claim.add_argument("--owner")
    claim.add_argument("--lease-minutes", type=int, default=120)
    claim.add_argument("--status")
    claim.add_argument("--type", default="function")
    claim.add_argument(
        "--module",
        help="claim the next target from an EMI/module filter, e.g. GAME#0",
    )
    claim.set_defaults(handler=run_claim)

    release = subparsers.add_parser("release", help="release a claim")
    release.add_argument("target_id")
    release.add_argument("--owner")
    release.set_defaults(handler=run_release)

    lift = subparsers.add_parser(
        "lift",
        help="initialize workspace, context, asm, and m2c draft for a target",
    )
    lift.add_argument("target_id")
    lift.add_argument("--m2c-arg", action="append", default=[])
    lift.set_defaults(handler=run_lift)

    verify = subparsers.add_parser("verify", help="run parity checks")
    verify_subs = verify.add_subparsers(required=True)
    verify_function = verify_subs.add_parser(
        "function",
        help="compile and asm-diff one function",
    )
    verify_function.add_argument("source_or_target")
    verify_function.add_argument("--allow-different", action="store_true")
    verify_function.set_defaults(handler=run_verify_function)
    verify_module = verify_subs.add_parser(
        "module",
        help="verify every source-backed function in a module",
    )
    verify_module.add_argument("module")
    verify_module.add_argument("--allow-different", action="store_true")
    verify_module.set_defaults(handler=run_verify_module)

    report = subparsers.add_parser("report", help="render harness reports")
    report_subs = report.add_subparsers(required=True)
    report_summary = report_subs.add_parser("summary", help="write summary reports")
    report_summary.set_defaults(handler=run_report_summary)
    report_module = report_subs.add_parser(
        "module",
        help="print per-module progress and match data",
    )
    report_module.add_argument("module")
    report_module.set_defaults(handler=run_report_module)
    report_function = report_subs.add_parser(
        "function",
        help="print lift and asm-diff paths for one function",
    )
    report_function.add_argument("target_or_source")
    report_function.set_defaults(handler=run_report_function)

    ghidra = subparsers.add_parser(
        "ghidra",
        help="serialized Ghidra project refresh commands",
    )
    ghidra_subs = ghidra.add_subparsers(required=True)
    ghidra_import = ghidra_subs.add_parser(
        "import-project",
        help="import manifest binaries through the harness Ghidra lock",
    )
    ghidra_import.add_argument("--owner")
    ghidra_import.add_argument("--lease-minutes", type=int, default=240)
    ghidra_import.add_argument("--ghidra-home", type=Path)
    ghidra_import.add_argument(
        "--manifest",
        type=Path,
        default=Path("out/ghidra/ghidra_import_manifest.json"),
    )
    ghidra_import.add_argument(
        "--project-dir",
        type=Path,
        default=Path("out/ghidra-project"),
    )
    ghidra_import.add_argument("--project-name", default="bof3_main")
    ghidra_import.add_argument(
        "--staging-dir",
        type=Path,
        default=Path("out/ghidra-import-staging"),
    )
    ghidra_import.add_argument("--script-path", type=Path)
    analyze_group = ghidra_import.add_mutually_exclusive_group()
    analyze_group.add_argument("--analyze", action="store_true")
    analyze_group.add_argument("--no-analysis", action="store_true")
    analyze_group.add_argument("--no-analyze", dest="no_analysis", action="store_true")
    ghidra_import.set_defaults(handler=run_ghidra_import_project)

    ghidra_analyze = ghidra_subs.add_parser(
        "analyze",
        help="run project analysis through the harness Ghidra lock",
    )
    ghidra_analyze.add_argument("--owner")
    ghidra_analyze.add_argument("--lease-minutes", type=int, default=240)
    ghidra_analyze.add_argument("--ghidra-home", type=Path)
    ghidra_analyze.add_argument(
        "--project-dir",
        type=Path,
        default=Path("out/ghidra-project"),
    )
    ghidra_analyze.add_argument("--project-name", default="bof3_main")
    ghidra_analyze.add_argument("--max-cpu", type=int)
    ghidra_analyze.set_defaults(handler=run_ghidra_analyze)

    ghidra_export = ghidra_subs.add_parser(
        "export",
        aliases=("export-symbols",),
        help="export Ghidra symbols through the harness Ghidra lock",
    )
    ghidra_export.add_argument("--owner")
    ghidra_export.add_argument("--lease-minutes", type=int, default=240)
    ghidra_export.add_argument("--ghidra-home", type=Path)
    ghidra_export.add_argument(
        "--project-dir",
        type=Path,
        default=Path("out/ghidra-project"),
    )
    ghidra_export.add_argument("--project-name", default="bof3_main")
    ghidra_export.add_argument(
        "--output",
        type=Path,
        default=Path("out/inventory/raw_ghidra_export.json"),
    )
    ghidra_export.add_argument(
        "--script-path",
        type=Path,
        default=Path("tools/ghidra/scripts/ExportAnalysisJson.java"),
    )
    ghidra_export.add_argument("--process", default="/")
    ghidra_export.add_argument("--no-recursive", action="store_true")
    ghidra_export.set_defaults(handler=run_ghidra_export)

    ghidra_coverage = ghidra_subs.add_parser(
        "coverage",
        help="compare import manifest programs to exported Ghidra symbols",
    )
    ghidra_coverage.add_argument("--allow-partial", action="store_true")
    ghidra_coverage.add_argument("--output", type=Path)
    ghidra_coverage.set_defaults(handler=run_ghidra_coverage)

    finish = subparsers.add_parser("finish", help="mark a function done")
    finish.add_argument("target_id")
    finish.add_argument(
        "--status",
        choices=("done", "blocked"),
        default="done",
    )
    finish.add_argument("--message", default="finished")
    finish.add_argument("--path", type=Path)
    finish.set_defaults(handler=run_finish)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="harness")
    configure_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
