"""``bin/harness text``: repository-owned BOF3 area dialogue text tooling."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

from harness.common.cli import add_example_argument
from harness.common.native import exec_tool
from harness.io import repo_layout


def tool_binary() -> str | None:
    """The tool's binary, built or refreshed when needed; ``None`` after an actionable message.

    Shared with `bin/harness text windows`, which needs the same freshness guarantee.
    """
    return _binary(repo_layout())


def _sources_newer(layout, binary: Path) -> bool:
    """Whether any crate source, manifest or lock file is newer than the built binary.

    The crate is binary-only, so `cargo test` builds only its test executable: an existing
    ``bof3-text`` file proves nothing about freshness. Treating a source-newer-than-binary state as
    needing a build keeps the harness from running gates against a stale executable.
    """
    built = binary.stat().st_mtime
    sources = [layout.bof3_text_src / "Cargo.toml", layout.bof3_text_src / "Cargo.lock"]
    sources += list((layout.bof3_text_src / "src").rglob("*.rs"))
    sources += list((layout.bof3_text_src / "data").rglob("*.json"))
    return any(path.is_file() and path.stat().st_mtime > built for path in sources)


def _binary(layout) -> str | None:
    """The tool path, building it once with cargo when it is missing.

    Mirrors the audio command's build-then-exec shape so a fresh checkout can run
    ``bin/harness text`` without a prior ``just setup``. Returns ``None`` after printing an
    actionable message when the build cannot be produced.
    """
    override = os.environ.get("PSX_BOF3_TEXT")
    if override:
        return override
    binary = layout.bof3_text_bin
    if binary.is_file() and not _sources_newer(layout, binary):
        return str(binary)
    if shutil.which("cargo") is None:
        print(
            "cargo: command not found; install a Rust toolchain, or run `just setup`",
            file=sys.stderr,
        )
        return None
    result = subprocess.run(
        [
            "cargo",
            "build",
            "--locked",
            "--release",
            "--manifest-path",
            str(layout.bof3_text_src / "Cargo.toml"),
            "--target-dir",
            str(binary.parent.parent),
        ],
        cwd=layout.root,
        capture_output=True,
        text=True,
    )
    if result.returncode or not binary.is_file():
        print(
            "bof3-text build failed: run `just setup`, or "
            "`cargo build --release --manifest-path tools/rust/bof3-text/Cargo.toml`, "
            "then retry",
            file=sys.stderr,
        )
        if result.stderr.strip():
            print(result.stderr.strip()[-800:], file=sys.stderr)
        return None
    return str(binary)


#: One canonical invocation per forwarded action, owned by the shared
#: ``--example`` mechanism rather than repeated in prose.
EXAMPLES = {
    "prepare": "bin/harness text prepare --root out/extracted/BIN",
    "payloads": "--root out/extracted/BIN",
    "vocabulary": "--out out/text-vocabulary/vocabulary.json",
    "windows": "--root out/extracted/BIN",
    "readability": "--text 'Worker CTTTT'",
    "index": "bin/harness text index out/extracted/BIN/WORLD00/AREA000.EMI",
    "scan": "bin/harness text scan out/extracted/BIN/WORLD00/AREA000.EMI --json",
    "extract": (
        "bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI "
        "-o out/text/AREA000.json"
    ),
    "validate": "bin/harness text validate out/text/AREA000.json",
    "pack": (
        "bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI "
        "--text out/text/AREA000.json -o out/text/AREA000.EMI"
    ),
    "query": "bin/harness text query out/extracted/BIN/WORLD00/AREA000.EMI --grep McNeil",
    "probe": (
        "bin/harness text probe out/extracted/BIN/WORLD00/AREA000.EMI --round-trip"
    ),
    "map": "bin/harness text map",
    "verify": "bin/harness text verify",
    "build-index": "bin/harness text build-index",
    "search": "bin/harness text search --grep spring",
}


def _requested_example(argv: list[str]) -> str | None:
    """Return the shared example for a forwarded action, or ``None``."""
    if not argv:
        return None
    example = EXAMPLES.get(argv[0])
    if example is None:
        return None
    parser = argparse.ArgumentParser(prog=f"bin/harness text {argv[0]}", add_help=False)
    add_example_argument(parser, example)
    return example if parser.parse_known_args(argv[1:])[0].example else None


def main(argv: list[str] | None = None) -> int:
    args = list(argv or [])
    example = _requested_example(args)
    if example is not None:
        print(example)
        return 0
    executable = _binary(repo_layout())
    if executable is None:
        return 2
    exec_tool(executable, args)
