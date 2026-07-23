"""Dispatch one repository-owned executable toolchain."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..io import repo_layout
from ..toolchain.maspsx import MaspsxToolchain
from ..toolchain.rizin import RizinToolchain
from ..toolchain.spimdisasm import SpimdisasmToolchain
from ._common import run_main


def _toolchain(root: Path, name: str):
    if name == "maspsx":
        return MaspsxToolchain(root)
    if name == "rizin":
        return RizinToolchain(repo_layout(root))
    if name == "spimdisasm":
        return SpimdisasmToolchain(root)
    raise ValueError(f"unknown toolchain executable: {name}")


def run(args: argparse.Namespace) -> int:
    toolchain = _toolchain(args.root.resolve(), args.name)
    if not toolchain.executable.is_file():
        raise FileNotFoundError(
            f"missing {toolchain.label} executable: {toolchain.executable}; run just setup"
        )
    python = getattr(toolchain, "python", None)
    if python is not None and not python.is_file():
        raise FileNotFoundError(f"missing project Python environment: {python}; run just setup")
    return toolchain.execute(args.arguments).returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="tool")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.add_argument("name", choices=("maspsx", "rizin", "spimdisasm"))
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
