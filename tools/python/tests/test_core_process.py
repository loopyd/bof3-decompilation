from __future__ import annotations

import sys
from pathlib import Path

import pytest

from rebof3.core import ProcessError, run_process


def test_run_process_captures_output_and_uses_cwd(tmp_path: Path) -> None:
    result = run_process(
        [
            sys.executable,
            "-c",
            "from pathlib import Path; print(Path.cwd().name)",
        ],
        cwd=tmp_path,
    )

    assert result.returncode == 0
    assert result.cwd == tmp_path
    assert result.stdout.strip() == tmp_path.name
    assert result.stderr == ""


def test_run_process_failure_reports_command_status_cwd_and_output(
    tmp_path: Path,
) -> None:
    with pytest.raises(ProcessError) as raised:
        run_process(
            [
                sys.executable,
                "-c",
                (
                    "import sys; "
                    "print('out text'); "
                    "print('err text', file=sys.stderr); "
                    "raise SystemExit(7)"
                ),
            ],
            cwd=tmp_path,
        )

    message = str(raised.value)
    assert "command failed with exit code 7" in message
    assert str(tmp_path) in message
    assert "out text" in message
    assert "err text" in message
    assert raised.value.result.returncode == 7
