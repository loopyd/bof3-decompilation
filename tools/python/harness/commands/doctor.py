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
from ..toolchain.setup_disc import find_disc_set
from ._common import run_main
from .setup import REQUIRED_TOOLS, _psyq_47_members


@dataclass(frozen=True)
class Check:
    label: str
    run: Callable[[Path], str]


def _require(root: Path, paths: tuple[Path, ...]) -> str:
    missing = [str(path.relative_to(root)) for path in paths if not path.exists()]
    if missing:
        raise FileNotFoundError(", ".join(missing))
    return f"{len(paths)} present"


def _toolchain(root: Path) -> str:
    layout = repo_layout(root)
    return _require(
        root,
        (
            layout.gcc272_psx_root / "gcc",
            layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-as",
            layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-ld",
            root / "third_party" / "maspsx" / "maspsx.py",
        ),
    )


def _psyq(root: Path) -> str:
    layout = repo_layout(root)
    _require(root, (layout.psyq_root / "include" / "libgpu.h", layout.psyq_root / "lib"))
    members = _psyq_47_members(root)
    _require(root, tuple(members))
    return f"headers, libraries, {len(members)} reviewed members"


def _disc(root: Path) -> str:
    cue, tracks = find_disc_set(root / "inputs" / "external")
    return f"{cue.name}, {len(tracks)} tracks"


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


def _tools(root: Path) -> str:
    layout = repo_layout(root)
    commands = (
        (root / "bin" / "cc", "-x", "c", "-E", "-"),
        *((root / tool, "--version") for tool in REQUIRED_TOOLS),
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


CHECKS = (
    Check("toolchain", _toolchain),
    Check("PsyQ 4.7", _psyq),
    Check("disc media", _disc),
    Check("target images", _target_images),
    Check("tool wrappers", _tools),
)


def _render(status: str, label: str, detail: str) -> None:
    print(f"[{status}] {label:<16} {detail}")


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    failed = 0
    for check in CHECKS:
        try:
            _render("PASS", check.label, check.run(root))
        except (FileNotFoundError, RuntimeError, ValueError, tomllib.TOMLDecodeError) as exc:
            failed += 1
            _render("FAIL", check.label, str(exc).replace("\n", "; "))
    print(f"doctor: {len(CHECKS) - failed}/{len(CHECKS)} checks passed")
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
