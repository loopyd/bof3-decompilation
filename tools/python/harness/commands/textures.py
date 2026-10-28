"""Build and dispatch the Rust BOF3 texture component."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import tomllib
from pathlib import Path

from harness.common.cli import add_example_argument, run_main
from harness.common.native import exec_tool
from harness.io import RepoLayout, repo_layout


def _read_channel(layout: RepoLayout) -> str:
    config = layout.bof3_textures_src / "rust-toolchain.toml"
    return tomllib.loads(config.read_text())["toolchain"]["channel"]


def build_binary(layout: RepoLayout) -> Path:
    rustup = shutil.which("rustup")
    if rustup is None:
        raise RuntimeError(
            "textures requires rustup and its declared installed Rust toolchain"
        )
    command = [
        rustup,
        "run",
        _read_channel(layout),
        "cargo",
        "build",
        "--offline",
        "--locked",
        "--release",
        "--manifest-path",
        str(layout.bof3_textures_src / "Cargo.toml"),
        "--target-dir",
        str(layout.bof3_textures_bin.parent.parent),
    ]
    result = subprocess.run(
        command, cwd=layout.root, check=False, capture_output=True, text=True
    )
    if result.returncode or not layout.bof3_textures_bin.is_file():
        raise RuntimeError("textures build failed: " + result.stderr.strip())
    return layout.bof3_textures_bin


def diagnose(layout: RepoLayout) -> str:
    binary = layout.bof3_textures_bin
    if not binary.is_file():
        raise RuntimeError("missing texture executable; run bin/harness textures build")
    inputs = [layout.root / "config/original-us-disc.json"]
    for root in (layout.bof3_textures_src, layout.emi_ex_src):
        inputs.extend((root / "src").rglob("*.rs"))
        inputs.extend(
            p
            for p in (
                root / "Cargo.toml",
                root / "Cargo.lock",
                root / "rust-toolchain.toml",
            )
            if p.is_file()
        )
    if any(p.stat().st_mtime_ns > binary.stat().st_mtime_ns for p in inputs):
        raise RuntimeError(
            "texture sources are newer than the executable; run bin/harness textures build"
        )
    missing = [
        name
        for name in ("rustup", "ffmpeg", "zip", "unzip")
        if shutil.which(name) is None
    ]
    if missing:
        raise RuntimeError("missing texture prerequisites: " + ", ".join(missing))
    for command in (
        ["rustup", "run", _read_channel(layout), "rustc", "--version"],
        [str(binary), "--help"],
    ):
        result = subprocess.run(
            command, cwd=layout.root, check=False, capture_output=True, text=True
        )
        if result.returncode:
            raise RuntimeError(
                result.stderr.strip() or "texture prerequisite check failed"
            )
    return f"{binary.relative_to(layout.root)}, Rust {_read_channel(layout)}, ffmpeg, zip, unzip"


def run_tool(args: argparse.Namespace) -> int:
    binary = build_binary(repo_layout())
    return exec_tool(str(binary), [args.operation, *args.arguments])


def run_build(args: argparse.Namespace) -> int:
    print(build_binary(repo_layout()))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness textures", add_help=False)
    add_example_argument(parser, "bin/harness textures categories --json")
    parser.add_argument(
        "operation",
        choices=(
            "categories",
            "index",
            "query",
            "unpack",
            "pack",
            "frame",
            "verify",
            "coverage",
            "package",
        ),
    )
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    parser.set_defaults(handler=run_tool)
    return parser


def build_command_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness textures build",
        description="Build the Rust texture tool with its installed pinned toolchain and cached dependencies.",
    )
    add_example_argument(parser, "bin/harness textures build")
    parser.set_defaults(handler=run_build)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


def build_main(argv: list[str] | None = None) -> int:
    return run_main(build_command_parser, argv)
