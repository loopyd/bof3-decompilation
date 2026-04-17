from __future__ import annotations

import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]


def test_configure_wrapper_uses_repo_root_for_presets(tmp_path: Path) -> None:
    result = subprocess.run(
        [str(REPO_ROOT / "bin" / "configure"), "--list-presets"],
        cwd=tmp_path,
        check=False,
        capture_output=True,
        text=True,
    )

    assert result.returncode == 0, result.stderr
    assert '"default"' in result.stdout


def test_build_wrapper_uses_repo_root_for_presets(tmp_path: Path) -> None:
    result = subprocess.run(
        [str(REPO_ROOT / "bin" / "build"), "--list-presets"],
        cwd=tmp_path,
        check=False,
        capture_output=True,
        text=True,
    )

    assert result.returncode == 0, result.stderr
    assert '"default"' in result.stdout
