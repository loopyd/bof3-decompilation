"""Contract tests for bin/python-env and converted wrappers."""

from __future__ import annotations

import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
HELP_WRAPPERS = (
    ("emi-target", "emi-target"),
    ("flag-search", "flag-search"),
    ("index", "index"),
    ("psyq-import", "psyq-import"),
    ("str-media", "str-media"),
)


def _run(*args: str, env: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=ROOT,
        capture_output=True,
        text=True,
        env=env,
        check=False,
    )


def test_python_helper_exports_project_paths() -> None:
    env = os.environ.copy()
    env.pop("PYTHONPATH", None)
    result = _run(
        "sh",
        "-c",
        f'ROOT="{ROOT}"; . "{ROOT}/bin/python-env"; printf "%s\\n%s\\n" "$PYTHON" "$PYTHONPATH"',
        env=env,
    )

    assert result.returncode == 0
    assert result.stdout.splitlines() == [
        str(ROOT / ".venv/bin/python"),
        str(ROOT / "tools/python"),
    ]


def test_python_helper_preserves_missing_python_exit(tmp_path: Path) -> None:
    env = os.environ | {"PSX_PYTHON": str(tmp_path / "missing-python")}
    result = _run("sh", "-c", f'. "{ROOT}/bin/python-env"', env=env)

    assert result.returncode == 2
    assert "missing project Python environment" in result.stderr


def test_converted_wrappers_forward_help() -> None:
    for wrapper, expected in HELP_WRAPPERS:
        result = _run(str(ROOT / "bin" / wrapper), "--help")
        assert result.returncode == 0, result.stderr
        assert expected in result.stdout


def test_converted_wrapper_preserves_missing_python_exit(tmp_path: Path) -> None:
    env = os.environ | {"PSX_PYTHON": str(tmp_path / "missing-python")}
    result = _run(str(ROOT / "bin" / "emi-target"), "--help", env=env)

    assert result.returncode == 2
    assert "missing project Python environment" in result.stderr
