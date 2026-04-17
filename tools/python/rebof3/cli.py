from __future__ import annotations

import argparse
from pathlib import Path
from typing import Callable

from .doctor import doctor_exit_code, render_doctor, run_doctor
from .inventory import group_exact_duplicates, scan_inventory
from .jsonio import read_json, write_json
from .models import DuplicateGroups, InventorySnapshot
from .paths import repo_layout
from .pipelines import run_ghidra_bootstrap_pipeline
from .planning import build_ghidra_manifest
from .setup import (
    SetupOptions,
    plan_setup_tasks,
    run_named_setup_task,
    run_setup_workspace,
    setup_task_names,
)
from .toolchain.aspsx import ALL_ASPSX_PSYQ_VERSIONS, download_aspsx_binaries
from .toolchain.setup_psyq import find_psyq_source, stage_psyq_sdk

CommandHandler = Callable[[argparse.Namespace], int]


def add_setup_option_flags(
    parser: argparse.ArgumentParser,
    *,
    include_force: bool = False,
    include_psyq_inputs: bool = False,
    include_skip_flags: bool = False,
) -> None:
    if include_psyq_inputs:
        parser.add_argument("--psyq-source-root", type=Path)
        parser.add_argument("--psyq-archive", type=Path)
    if include_force:
        parser.add_argument("--force", action="store_true")
    if include_skip_flags:
        parser.add_argument("--skip-aspsx-binaries", action="store_true")
        parser.add_argument("--skip-match-tools", action="store_true")
        parser.add_argument("--skip-extract", action="store_true")
        parser.add_argument("--skip-ghidra-plan", action="store_true")


def build_setup_options(args: argparse.Namespace) -> SetupOptions:
    return SetupOptions(
        force=bool(getattr(args, "force", False)),
        include_aspsx_binaries=not bool(getattr(args, "skip_aspsx_binaries", False)),
        include_match_tools=not bool(getattr(args, "skip_match_tools", False)),
        include_extract=not bool(getattr(args, "skip_extract", False)),
        include_ghidra_plan=not bool(getattr(args, "skip_ghidra_plan", False)),
        psyq_source_root=getattr(args, "psyq_source_root", None),
        psyq_archive=getattr(args, "psyq_archive", None),
    )


def run_inventory_scan(args: argparse.Namespace) -> int:
    snapshot = scan_inventory(
        slus_path=args.slus,
        logo_path=args.logo,
        emi_root=args.emi_root,
    )
    write_json(args.output, snapshot.to_dict())
    print(f"wrote {len(snapshot.programs)} programs to {args.output}")
    return 0


def run_inventory_group(args: argparse.Namespace) -> int:
    snapshot = InventorySnapshot.from_dict(read_json(args.input))
    groups = group_exact_duplicates(snapshot)
    write_json(args.output, groups.to_dict())
    print(f"wrote {len(groups.groups)} duplicate groups to {args.output}")
    return 0


def run_plan_ghidra(args: argparse.Namespace) -> int:
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


def run_pipeline_ghidra_bootstrap(args: argparse.Namespace) -> int:
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


def run_ghidra_summary(args: argparse.Namespace) -> int:
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


def run_doctor_command(args: argparse.Namespace) -> int:
    checks = run_doctor(layout=repo_layout())
    render_doctor(checks)
    return doctor_exit_code(checks, strict=bool(getattr(args, "strict", False)))


def run_toolchain_psyq_find(args: argparse.Namespace) -> int:
    source = find_psyq_source(source_root=args.source_root, archive=args.archive)
    if source is None:
        print("not found")
        return 1
    print(f"{source.kind}: {source.path}")
    return 0


def run_toolchain_psyq_setup(args: argparse.Namespace) -> int:
    dest = stage_psyq_sdk(
        dest=args.dest,
        source_root=args.source_root,
        archive=args.archive,
        force=args.force,
    )
    print(f"staged: {dest}")
    return 0


def run_toolchain_aspsx_download(args: argparse.Namespace) -> int:
    result = download_aspsx_binaries(
        repo_layout(),
        versions=ALL_ASPSX_PSYQ_VERSIONS if args.all_versions else None,
        force=args.force,
    )
    print(f"downloaded: {result.root}")
    print(f"versions: {', '.join(result.versions)}")
    return 0


def run_setup_plan(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    for task in plan_setup_tasks(options):
        print(f"{task.name}: {task.description}")
    return 0


def run_setup_workspace_command(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    run_setup_workspace(options)
    print("workspace setup complete")
    return 0


def run_setup_task_command(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    run_named_setup_task(args.task_name, options)
    print(f"setup task complete: {args.task_name}")
    return 0


def add_inventory_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    inventory = subparsers.add_parser("inventory")
    inventory_subparsers = inventory.add_subparsers(required=True)

    inventory_scan = inventory_subparsers.add_parser("scan")
    inventory_scan.add_argument("--slus", type=Path)
    inventory_scan.add_argument("--logo", type=Path)
    inventory_scan.add_argument("--emi-root", type=Path)
    inventory_scan.add_argument("--output", type=Path, required=True)
    inventory_scan.set_defaults(handler=run_inventory_scan)

    inventory_group = inventory_subparsers.add_parser("group")
    inventory_group.add_argument("--input", type=Path, required=True)
    inventory_group.add_argument("--output", type=Path, required=True)
    inventory_group.set_defaults(handler=run_inventory_group)


def add_plan_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    plan = subparsers.add_parser("plan")
    plan_subparsers = plan.add_subparsers(required=True)

    plan_ghidra = plan_subparsers.add_parser("ghidra")
    plan_ghidra.add_argument("--inventory", type=Path, required=True)
    plan_ghidra.add_argument("--groups", type=Path)
    plan_ghidra.add_argument("--output", type=Path, required=True)
    plan_ghidra.add_argument("--no-analyze", action="store_true")
    plan_ghidra.set_defaults(handler=run_plan_ghidra)


def add_pipeline_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    pipeline = subparsers.add_parser("pipeline")
    pipeline_subparsers = pipeline.add_subparsers(required=True)

    pipeline_ghidra = pipeline_subparsers.add_parser("ghidra-bootstrap")
    pipeline_ghidra.add_argument("--slus", type=Path)
    pipeline_ghidra.add_argument("--logo", type=Path)
    pipeline_ghidra.add_argument("--emi-root", type=Path)
    pipeline_ghidra.add_argument("--output-dir", type=Path, required=True)
    pipeline_ghidra.add_argument("--no-analyze", action="store_true")
    pipeline_ghidra.set_defaults(handler=run_pipeline_ghidra_bootstrap)


def add_ghidra_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    ghidra = subparsers.add_parser("ghidra")
    ghidra_subparsers = ghidra.add_subparsers(required=True)

    ghidra_summary = ghidra_subparsers.add_parser("summary")
    ghidra_summary.add_argument("--input", type=Path, required=True)
    ghidra_summary.set_defaults(handler=run_ghidra_summary)


def add_doctor_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    doctor = subparsers.add_parser("doctor")
    doctor.add_argument("--strict", action="store_true")
    doctor.set_defaults(handler=run_doctor_command)


def add_toolchain_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    toolchain = subparsers.add_parser("toolchain")
    toolchain_subparsers = toolchain.add_subparsers(required=True)

    toolchain_psyq = toolchain_subparsers.add_parser("psyq")
    toolchain_psyq_subparsers = toolchain_psyq.add_subparsers(required=True)

    toolchain_psyq_find = toolchain_psyq_subparsers.add_parser("find")
    toolchain_psyq_find.add_argument("--source-root", type=Path)
    toolchain_psyq_find.add_argument("--archive", type=Path)
    toolchain_psyq_find.set_defaults(handler=run_toolchain_psyq_find)

    toolchain_psyq_setup = toolchain_psyq_subparsers.add_parser("setup")
    toolchain_psyq_setup.add_argument("--source-root", type=Path)
    toolchain_psyq_setup.add_argument("--archive", type=Path)
    toolchain_psyq_setup.add_argument("--dest", type=Path, required=True)
    toolchain_psyq_setup.add_argument("--force", action="store_true")
    toolchain_psyq_setup.set_defaults(handler=run_toolchain_psyq_setup)

    toolchain_aspsx = toolchain_subparsers.add_parser("aspsx")
    toolchain_aspsx_subparsers = toolchain_aspsx.add_subparsers(required=True)

    toolchain_aspsx_download = toolchain_aspsx_subparsers.add_parser("download")
    toolchain_aspsx_download.add_argument("--all-versions", action="store_true")
    toolchain_aspsx_download.add_argument("--force", action="store_true")
    toolchain_aspsx_download.set_defaults(handler=run_toolchain_aspsx_download)


def add_setup_commands(
    subparsers: argparse._SubParsersAction[argparse.ArgumentParser],
) -> None:
    setup = subparsers.add_parser("setup")
    setup_subparsers = setup.add_subparsers(required=True)

    setup_plan = setup_subparsers.add_parser("plan")
    add_setup_option_flags(setup_plan, include_skip_flags=True)
    setup_plan.set_defaults(handler=run_setup_plan)

    setup_workspace = setup_subparsers.add_parser("workspace")
    add_setup_option_flags(
        setup_workspace,
        include_force=True,
        include_psyq_inputs=True,
        include_skip_flags=True,
    )
    setup_workspace.set_defaults(handler=run_setup_workspace_command)

    setup_task = setup_subparsers.add_parser("task")
    setup_task.add_argument("task_name", choices=setup_task_names())
    add_setup_option_flags(setup_task, include_force=True, include_psyq_inputs=True)
    setup_task.set_defaults(handler=run_setup_task_command)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bof3")
    subparsers = parser.add_subparsers(required=True)

    add_inventory_commands(subparsers)
    add_plan_commands(subparsers)
    add_pipeline_commands(subparsers)
    add_ghidra_commands(subparsers)
    add_doctor_commands(subparsers)
    add_toolchain_commands(subparsers)
    add_setup_commands(subparsers)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    handler = getattr(args, "handler", None)
    if handler is None:
        parser.error("missing command handler")
    return handler(args)


if __name__ == "__main__":
    raise SystemExit(main())
