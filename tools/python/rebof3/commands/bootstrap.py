from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..core import Pipeline
from ..core.process import ProcessError
from ..paths import repo_layout
from ..tasks import CommandExecutor, CommandTaskSpec, build_command_task
from ..tasks import run_workspace_command
from ._common import run_main
from .pipeline import print_pipeline_plan, print_pipeline_result


def _bin(root: Path, name: str) -> str:
    return str(root / "bin" / name)


def _task(
    *,
    root: Path,
    executor: CommandExecutor,
    name: str,
    description: str,
    command: tuple[str, ...],
):
    return build_command_task(
        CommandTaskSpec(
            name=name,
            description=description,
            commands=(command,),
        ),
        root=root,
        executor=executor,
    )


def build_bootstrap_pipeline(
    *,
    root: Path | None = None,
    executor: CommandExecutor = run_workspace_command,
) -> Pipeline:
    repo_root = root or repo_layout().root
    harness = _bin(repo_root, "harness")
    return Pipeline(
        name="bootstrap",
        description="Prepare extraction, inventory, Ghidra, and harness state",
        tasks=[
            _task(
                root=repo_root,
                executor=executor,
                name="doctor",
                description="Check workspace dependencies and environment",
                command=(_bin(repo_root, "doctor"), "--profile", "workspace"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="setup",
                description="Install and build maintained workspace dependencies",
                command=(_bin(repo_root, "setup"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="disk-extract",
                description="Extract BOF3 disc files from inputs/disc",
                command=(_bin(repo_root, "disk-extract"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="emi-unpack",
                description="Unpack extracted EMI archives inline into out/extracted",
                command=(_bin(repo_root, "emi-unpack"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="inventory-build",
                description="Build maintained inventory artifacts",
                command=(_bin(repo_root, "inventory-build"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-bootstrap",
                description="Build the Ghidra import manifest",
                command=(_bin(repo_root, "ghidra-bootstrap"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-import-project",
                description="Import binaries into the serialized Ghidra project lane",
                command=(harness, "ghidra", "import-project", "--no-analysis"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-analyze",
                description="Analyze the imported Ghidra project",
                command=(harness, "ghidra", "analyze"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="ghidra-export",
                description="Export Ghidra symbols into out/inventory",
                command=(harness, "ghidra", "export"),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="inventory-import-ghidra-symbols",
                description="Refresh function indexes from the raw Ghidra export",
                command=(_bin(repo_root, "inventory-import-ghidra-symbols"),),
            ),
            _task(
                root=repo_root,
                executor=executor,
                name="harness-refresh",
                description="Refresh harness state and reports",
                command=(harness, "refresh"),
            ),
        ],
    )


def run_bootstrap_command(args: argparse.Namespace) -> int:
    pipeline = build_bootstrap_pipeline()
    if args.plan:
        print_pipeline_plan(pipeline)
        return 0
    try:
        result = pipeline.run()
    except ProcessError as exc:
        print(str(exc), file=sys.stderr)
        return exc.result.returncode
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    print_pipeline_result(result)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bootstrap")
    parser.add_argument("--plan", action="store_true", help="print the task plan")
    parser.set_defaults(handler=run_bootstrap_command)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
