"""Validate that the local reverse-engineering setup is ready to use."""

from __future__ import annotations

import argparse
import subprocess
import tomllib
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

from ..domain import load_target_manifests
from ..io import repo_layout
from ..toolchain.disc import DiscToolchain
from ..toolchain.gcc import GccToolchain
from ..toolchain.maspsx import MaspsxToolchain
from ..toolchain.psn00b import Psn00bToolchain
from ..toolchain.psyq import PsyqToolchain
from ..toolchain.asm_differ import AsmDifferToolchain
from ..toolchain.m2c import M2cToolchain
from ..toolchain.permuter import DecompPermuterToolchain
from ..toolchain.rizin import RizinToolchain
from ..toolchain.signatures import PsyqSignaturesToolchain
from ._common import run_main
from .setup import REQUIRED_TOOLS, _psyq_47_members


Task = Callable[[Path], str]


@dataclass(frozen=True)
class DoctorTask:
    label: str
    run: Task


TASKS: list[DoctorTask] = []


def doctor_task(label: str) -> Callable[[Task], Task]:
    def register(run: Task) -> Task:
        TASKS.append(DoctorTask(label, run))
        return run

    return register


def _require(root: Path, paths: tuple[Path, ...]) -> str:
    missing = [str(path.relative_to(root)) for path in paths if not path.exists()]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(paths)} present"


@doctor_task("toolchain")
def _toolchain(root: Path) -> str:
    layout = repo_layout(root)
    return ", ".join(
        toolchain.verify()
        for toolchain in (
            Psn00bToolchain(layout),
            GccToolchain(layout),
            MaspsxToolchain(root),
            RizinToolchain(layout),
            M2cToolchain(root),
            AsmDifferToolchain(root),
            DecompPermuterToolchain(root),
            PsyqSignaturesToolchain(root),
        )
    )


@doctor_task("PsyQ 4.7")
def _psyq(root: Path) -> str:
    layout = repo_layout(root)
    PsyqToolchain(layout).verify()
    members = _psyq_47_members(root)
    _require(root, tuple(members))
    return f"headers, libraries, {len(members)} reviewed members"


@doctor_task("disc media")
def _disc(root: Path) -> str:
    return DiscToolchain(root).verify()


@doctor_task("target images")
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


@doctor_task("tool wrappers")
def _tools(root: Path) -> str:
    layout = repo_layout(root)
    commands = (
        (root / "bin" / "cc", "-x", "c", "-E", "-"),
        *((root / tool, "--version") for tool in REQUIRED_TOOLS),
        (root / "bin" / "rizin", "-V"),
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
            raise RuntimeError(f"{' '.join(map(str, command))} exited {result.returncode}")
    return f"{len(commands)} commands"


def _render(status: str, label: str, detail: str) -> None:
    print(f"[{status}] {label:<{max(len(task.label) for task in TASKS)}}  {detail}")


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    failed = 0
    for task in TASKS:
        try:
            _render("PASS", task.label, task.run(root))
        except (FileNotFoundError, RuntimeError, ValueError, tomllib.TOMLDecodeError) as exc:
            failed += 1
            _render("FAIL", task.label, str(exc).replace("\n", "; "))
    print(f"doctor: {len(TASKS) - failed}/{len(TASKS)} checks passed")
    return 2 if failed else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="doctor")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
