"""Build, invoke and package the Rust BOF3 audio tools."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import zipfile

from harness.common.cli import add_example_argument, run_main
from harness.common.native import exec_tool
from harness.io import repo_layout

OPERATIONS = ("index", "query", "map", "extract", "render", "pack", "verify")
CRATES = ("bof3-audio", "emi-ex")
PACKAGE_ROOT = "bof3-audio"


def build_binary() -> Path | None:
    """Build local crate inputs with the audio project's mise toolchain."""
    mise = shutil.which("mise")
    if mise is None:
        print(
            "bof3-audio build requires mise and the project Rust toolchain",
            file=sys.stderr,
        )
        return None
    root = repo_layout().root
    config = root / "tools/rust/bof3-audio/mise.toml"
    if not config.is_file():
        print(
            f"bof3-audio build requires its project toolchain config: {config}",
            file=sys.stderr,
        )
        return None
    target = root / "out" / "audio"
    binary = target / "release" / "bof3-audio"
    result = subprocess.run(
        [
            mise,
            "-C",
            str(root / "tools/rust/bof3-audio"),
            "exec",
            "--",
            "cargo",
            "build",
            "--locked",
            "--release",
            "--manifest-path",
            str(root / "tools/rust/bof3-audio/Cargo.toml"),
            "--target-dir",
            str(target),
        ],
        cwd=root,
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0 or not binary.is_file():
        sys.stdout.write(result.stdout)
        sys.stderr.write(result.stderr)
        print(
            "bof3-audio build failed: inspect mise/Cargo diagnostics and the project toolchain",
            file=sys.stderr,
        )
        return None
    return binary


def run_tool(args: argparse.Namespace) -> int:
    binary = build_binary()
    if binary is None:
        return 2
    return exec_tool(str(binary), [args.operation, *args.arguments])


def run_build(args: argparse.Namespace) -> int:
    binary = build_binary()
    if binary is None:
        return 2
    if args.arguments:
        return exec_tool(str(binary), args.arguments)
    print(binary)
    return 0


def collect_sources(root: Path) -> list[Path]:
    """Allow only authored crate inputs, never media, binaries or build caches."""
    files: list[Path] = []
    for name in CRATES:
        crate = root / "tools/rust" / name
        if crate.is_symlink():
            raise ValueError(f"source crate is a symlink: {crate}")
        required_files = ("Cargo.toml", "Cargo.lock")
        if name == "bof3-audio":
            required_files += ("mise.toml",)
        for required in required_files:
            source = crate / required
            if not source.is_file() or source.is_symlink():
                raise ValueError(f"missing or unsafe package input: {source}")
            files.append(source)
        for directory in ("src", "tests", "examples"):
            base = crate / directory
            if base.is_symlink():
                raise ValueError(f"source directory is a symlink: {base}")
            if directory == "src" and not base.is_dir():
                raise ValueError(f"missing source directory: {base}")
            for source in sorted(base.rglob("*")):
                if source.is_symlink():
                    raise ValueError(f"source input is a symlink: {source}")
                if source.is_file() and source.suffix == ".py":
                    raise ValueError(
                        f"Python belongs in harness integration tests, not the audio crate: {source}"
                    )
                if source.is_file() and source.suffix in {".rs", ".json"}:
                    files.append(source)
        for source in sorted(crate.iterdir()):
            if source.name.startswith(
                ("LICENSE", "NOTICE", "THIRD_PARTY_LICENSES", "README")
            ):
                if source.is_symlink() or not source.is_file():
                    raise ValueError(f"unsafe documentation/license input: {source}")
                files.append(source)
    for required in ("README.md", "THIRD_PARTY_LICENSES.txt"):
        if root / "tools/rust/bof3-audio" / required not in files:
            raise ValueError(f"missing bof3-audio package {required}")
    return sorted(files)


def write_package(root: Path, output: Path) -> None:
    """Publish a complete ZIP atomically; failures preserve an existing output."""
    sources = collect_sources(root)
    if output.is_symlink():
        raise ValueError(f"package output is a symlink: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    staged: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            dir=output.parent, prefix=".bof3-audio-", suffix=".zip", delete=False
        ) as stream:
            staged = Path(stream.name)
        with zipfile.ZipFile(staged, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            for source in sources:
                name = f"{PACKAGE_ROOT}/{source.relative_to(root).as_posix()}"
                entry = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
                entry.compress_type = zipfile.ZIP_DEFLATED
                entry.external_attr = 0o100644 << 16
                archive.writestr(entry, source.read_bytes())
        with zipfile.ZipFile(staged) as archive:
            if archive.testzip() is not None:
                raise ValueError("source package CRC verification failed")
        os.replace(staged, output)
    finally:
        if staged is not None:
            staged.unlink(missing_ok=True)


def run_package(args: argparse.Namespace) -> int:
    root = repo_layout().root
    output = args.output or root / "out/bof3-audio-source.zip"
    try:
        write_package(root, output)
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        print(f"bof3-audio package failed: {error}", file=sys.stderr)
        return 2
    print(f"wrote {output}")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness audio", add_help=False)
    add_example_argument(
        parser, "bin/harness audio index --mode music --disc-root out/extracted --json"
    )
    parser.add_argument("operation", choices=OPERATIONS)
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    parser.set_defaults(handler=run_tool)
    return parser


def build_command_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness audio build",
        description="Build Rust audio; optionally run OPERATION and its arguments.",
    )
    add_example_argument(parser, "bin/harness audio build")
    parser.add_argument("arguments", nargs=argparse.REMAINDER)
    parser.set_defaults(handler=run_build)
    return parser


def package_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness audio package")
    add_example_argument(parser, "bin/harness audio package out/bof3-audio-source.zip")
    parser.add_argument("output", nargs="?", type=Path)
    parser.set_defaults(handler=run_package)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


def build_main(argv: list[str] | None = None) -> int:
    return run_main(build_command_parser, argv)


def package_main(argv: list[str] | None = None) -> int:
    return run_main(package_parser, argv)
