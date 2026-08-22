"""Validate that the local reverse-engineering setup is ready to use."""

from __future__ import annotations

import argparse
import subprocess
import tomllib
from pathlib import Path

from ..build.compiler import load_object_compilers
from ..domain import load_target_manifests
from ..io import repo_layout
from ..toolchain import managed_lifecycle
from ..toolchain.disc import DiscToolchain
from ..toolchain.psyq import PsyqToolchain
from ._common import (
    Check,
    add_root_argument,
    register_check,
    render_task,
    run_main,
)
from .setup import REQUIRED_TOOLS, _psyq_47_members


TASKS: list[Check[Path]] = []


def _require(root: Path, paths: tuple[Path, ...]) -> str:
    missing = [str(path.relative_to(root)) for path in paths if not path.exists()]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(paths)} present"


@register_check("toolchain", TASKS)
def _toolchain(root: Path) -> str:
    from ..toolchain.gcc_variants import lookup_variant

    layout = repo_layout(root)
    labels = list(managed_lifecycle(layout, verify_only=True))

    # Inspect compiler variants only when BOF3_OBJCOMPILER_ selections exist.
    selections = load_object_compilers(root)
    if selections:
        verified_ids = set()
        for key, cid in selections.items():
            if cid in verified_ids:
                continue
            verified_ids.add(cid)
            variant = lookup_variant(layout, cid)
            variant.verify(layout)
            labels.append(f"compiler={variant.label} ({variant.id})")

    return ", ".join(labels)


@register_check("PsyQ 4.7", TASKS)
def _psyq(root: Path) -> str:
    layout = repo_layout(root)
    PsyqToolchain(layout).verify()
    members = _psyq_47_members(root)
    _require(root, tuple(members))
    return f"headers, libraries, {len(members)} reviewed members"


@register_check("disc media", TASKS)
def _disc(root: Path) -> str:
    return DiscToolchain(repo_layout(root)).verify()


@register_check("target images", TASKS)
def _target_images(root: Path) -> str:
    manifests = load_target_manifests(root)
    missing = [
        str(manifest.binary)
        for manifest in manifests.values()
        if not (root / manifest.binary).is_file()
    ]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(manifests)} images"


@register_check("tool wrappers", TASKS)
def _tools(root: Path) -> str:
    layout = repo_layout(root)
    commands = (
        (root / "bin" / "cc", "-x", "c", "-E", "-"),
        *((root / tool, "--version") for tool in REQUIRED_TOOLS),
        (root / "bin" / "rizin", "-V"),
        (root / "bin" / "maspsx", "--help"),
        (root / "bin" / "spimdisasm", "--version"),
        (layout.harness_disk_bin, "--help"),
        (layout.emi_ex_bin, "--help"),
    )
    for command in commands:
        result = subprocess.run(
            [str(part) for part in command],
            cwd=root,
            stdin=None if command[0] == root / "bin" / "cc" else subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if result.returncode:
            raise RuntimeError(
                f"{' '.join(map(str, command))} exited {result.returncode}"
            )
    return f"{len(commands)} commands"


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    failed = 0
    for task in TASKS:
        try:
            render_task("PASS", task.label, task.run(root), TASKS)
        except (
            FileNotFoundError,
            RuntimeError,
            ValueError,
            tomllib.TOMLDecodeError,
        ) as exc:
            failed += 1
            render_task("FAIL", task.label, str(exc).replace("\n", "; "), TASKS)
    print(f"doctor: {len(TASKS) - failed}/{len(TASKS)} checks passed")
    return 2 if failed else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="doctor")
    add_root_argument(parser)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
