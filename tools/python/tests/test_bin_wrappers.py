from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[3]


def run_wrapper(*args: str, cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(REPO_ROOT / "bin" / args[0]), *args[1:]],
        cwd=cwd,
        check=False,
        capture_output=True,
        text=True,
    )


@pytest.mark.parametrize("command", ["configure", "build"])
def test_wrapper_list_presets(command: str, tmp_path: Path) -> None:
    result = run_wrapper(command, "--list-presets", cwd=tmp_path)
    assert result.returncode == 0, result.stderr
    assert '"default"' in result.stdout


_HELP_COMMANDS = [
    ("disk-extract", "usage: disk-extract"),
    ("emi-unpack", "usage: emi-unpack"),
    ("emi-extract", "usage: emi-extract"),
    ("emi-review", "usage: emi-review"),
    ("emi-extract-archive", "usage: emi-extract-archive"),
    ("emi-extract-tree", "usage: emi-extract-tree"),
    ("emi-render-title", "usage: emi-render-title"),
    ("emi-render-status", "usage: emi-render-status"),
    ("emi-preview", "usage: emi-preview"),
    ("inventory-build", "usage: inventory build"),
    ("inventory-import-ghidra-symbols", "usage: inventory import-ghidra-symbols"),
    ("ghidra-install-extensions", "usage: ghidra install-extensions"),
    ("ghidra-import-project", "usage: ghidra import-project"),
    ("ghidra-export-symbols", "usage: ghidra export-symbols"),
    ("pipeline", "usage: pipeline"),
    ("download-psyq", "usage: download-psyq"),
    ("match-init", "usage: match init"),
    ("match-report", "usage: match report"),
    ("harness", "usage: harness"),
    ("harness verify", "usage: harness verify"),
    ("harness verify function", "usage: harness verify function"),
]


@pytest.mark.parametrize("args,usage_text", _HELP_COMMANDS)
def test_wrapper_help(args: str, usage_text: str, tmp_path: Path) -> None:
    result = run_wrapper(*args.split(), "--help", cwd=tmp_path)
    assert result.returncode == 0, result.stderr
    assert usage_text in result.stdout
