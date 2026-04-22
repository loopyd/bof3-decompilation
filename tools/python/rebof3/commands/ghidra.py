from __future__ import annotations

import argparse
from pathlib import Path

from ..ghidra import (
    DEFAULT_PROJECT_NAME,
    DEFAULT_PROJECT_ROOT,
    install_extensions,
    launch_ui,
)
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


def run_ui(args: argparse.Namespace) -> int:
    return launch_ui(
        ghidra_home=args.ghidra_home,
        project_dir=args.project_dir,
        project_name=args.project_name,
        extra_args=args.ghidra_arg,
    )


def run_install_extensions(args: argparse.Namespace) -> int:
    extensions_dir, installed_paths = install_extensions(
        args.sources,
        user_dir=args.user_dir,
    )
    print(f"extensions-dir: {extensions_dir}")
    for path in installed_paths:
        print(f"installed: {path}")
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


def configure_ui_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--ghidra-home", type=Path)
    parser.add_argument("--project-dir", type=Path, default=DEFAULT_PROJECT_ROOT)
    parser.add_argument("--project-name", default=DEFAULT_PROJECT_NAME)
    parser.add_argument("--ghidra-arg", action="append", default=[])
    parser.set_defaults(handler=run_ui)


def configure_install_extensions_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("sources", nargs="+", type=Path)
    parser.add_argument("--user-dir", type=Path)
    parser.set_defaults(handler=run_install_extensions)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    bootstrap = subparsers.add_parser("bootstrap")
    configure_bootstrap_parser(bootstrap)

    plan = subparsers.add_parser("plan")
    configure_plan_parser(plan)

    summary = subparsers.add_parser("summary")
    configure_summary_parser(summary)

    ui = subparsers.add_parser("ui")
    configure_ui_parser(ui)

    install_extensions_parser = subparsers.add_parser("install-extensions")
    configure_install_extensions_parser(install_extensions_parser)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="ghidra")
    configure_root_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
