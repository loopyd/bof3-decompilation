from __future__ import annotations

import argparse
from pathlib import Path

from ..jsonio import read_json, write_json
from ..models import DuplicateGroups, InventorySnapshot
from ..paths import repo_layout
from ..pipelines import run_ghidra_bootstrap_pipeline
from ..planning import build_ghidra_manifest
from ._common import run_main


def run_bootstrap(args: argparse.Namespace) -> int:
    outputs = run_ghidra_bootstrap_pipeline(
        slus_path=args.slus,
        logo_path=args.logo,
        emi_root=args.emi_root,
        output_dir=args.output_dir,
        analyze=not args.no_analyze,
    )
    print(f"inventory: {outputs['inventory']}")
    print(f"groups: {outputs['groups']}")
    print(f"manifest: {outputs['manifest']}")
    return 0


def run_plan(args: argparse.Namespace) -> int:
    snapshot = InventorySnapshot.from_dict(read_json(args.inventory))
    groups = DuplicateGroups.from_dict(read_json(args.groups)) if args.groups else None
    manifest = build_ghidra_manifest(
        snapshot,
        groups,
        analyze=not args.no_analyze,
    )
    write_json(args.output, manifest.to_dict())
    print(f"wrote {len(manifest.imports)} imports to {args.output}")
    return 0


def run_summary(args: argparse.Namespace) -> int:
    payload = read_json(args.input)
    imports = payload.get("imports", [])
    if not isinstance(imports, list):
        raise ValueError("manifest imports must be a list")

    boot_count = 0
    overlay_count = 0
    for entry in imports:
        if not isinstance(entry, dict):
            continue
        folder = str(entry.get("project_folder_path") or "")
        if folder == "/boot":
            boot_count += 1
        else:
            overlay_count += 1

    print(f"imports: {len(imports)}")
    print(f"boot: {boot_count}")
    print(f"overlays: {overlay_count}")
    print(f"analyze: {bool(payload.get('analyze', False))}")
    return 0


def configure_bootstrap_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--slus", type=Path, default=layout.slus_path)
    parser.add_argument("--logo", type=Path, default=layout.logo_path)
    parser.add_argument("--emi-root", type=Path, default=layout.emi_root)
    parser.add_argument("--output-dir", type=Path, default=layout.ghidra_bootstrap_dir)
    parser.add_argument("--no-analyze", action="store_true")
    parser.set_defaults(handler=run_bootstrap)


def configure_plan_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--inventory", type=Path, default=layout.inventory_path)
    parser.add_argument("--groups", type=Path, default=layout.groups_path)
    parser.add_argument("--output", type=Path, default=layout.ghidra_manifest_path)
    parser.add_argument("--no-analyze", action="store_true")
    parser.set_defaults(handler=run_plan)


def configure_summary_parser(parser: argparse.ArgumentParser) -> None:
    layout = repo_layout()
    parser.add_argument("--input", type=Path, default=layout.ghidra_manifest_path)
    parser.set_defaults(handler=run_summary)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    bootstrap = subparsers.add_parser("bootstrap")
    configure_bootstrap_parser(bootstrap)

    plan = subparsers.add_parser("plan")
    configure_plan_parser(plan)

    summary = subparsers.add_parser("summary")
    configure_summary_parser(summary)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="ghidra")
    configure_root_parser(parser)
    return parser


def add_legacy_plan_parser(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    parser = subparsers.add_parser("ghidra")
    configure_plan_parser(parser)


def add_legacy_pipeline_parser(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    parser = subparsers.add_parser("ghidra-bootstrap")
    configure_bootstrap_parser(parser)


def add_legacy_ghidra_parser(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    parser = subparsers.add_parser("summary")
    configure_summary_parser(parser)


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
