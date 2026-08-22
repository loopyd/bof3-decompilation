"""Dispatch one repository-owned executable toolchain."""

from __future__ import annotations

import argparse

from ..io import repo_layout
from ..toolchain import managed_toolchain
from ._common import add_root_argument, run_main


def run(args: argparse.Namespace) -> int:
    layout = repo_layout(args.root.resolve())
    toolchain = managed_toolchain(layout, args.name)
    if not toolchain.executable.is_file():
        raise FileNotFoundError(
            f"missing {toolchain.label} executable: {toolchain.executable}; run just setup"
        )
    python = getattr(toolchain, "python", None)
    if python is not None and not python.is_file():
        raise FileNotFoundError(
            f"missing project Python environment: {python}; run just setup"
        )
    return toolchain.execute(args.arguments).returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="tool")
    add_root_argument(parser)
    parser.add_argument("name", choices=("maspsx", "rizin", "spimdisasm"))
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
