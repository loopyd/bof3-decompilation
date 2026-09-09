"""Crash-safety tests for analyzer subprocess ownership."""

from __future__ import annotations

import os
import signal
import subprocess
import sys
import time
from pathlib import Path

import pytest


def _process_state(pid: int) -> tuple[str, int] | None:
    try:
        stat = Path(f"/proc/{pid}/stat").read_text()
        fields = stat[stat.rindex(")") + 2 :].split()
        return fields[0], int(fields[19])
    except (
        FileNotFoundError,
        ProcessLookupError,
        PermissionError,
        OSError,
        ValueError,
    ):
        return None


def _alive(pid: int, starttime: int) -> bool:
    state = _process_state(pid)
    return state is not None and state[1] == starttime and state[0] != "Z"


def _read_pid(path: Path, deadline: float) -> int:
    while time.monotonic() < deadline:
        try:
            return int(path.read_text())
        except (FileNotFoundError, ValueError):
            time.sleep(0.02)
    raise AssertionError(f"PID file was not populated: {path}")


def _wait_not_alive(
    pids: list[tuple[int, int]], deadline: float, check=_alive
) -> tuple[bool, ...]:
    previous: tuple[bool, ...] | None = None
    while time.monotonic() < deadline:
        observed = tuple(check(*process) for process in pids)
        if observed == previous and not any(observed):
            return observed
        previous = observed
        time.sleep(0.02)
    return previous or tuple(check(*process) for process in pids)


def test_alive_handles_process_states(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr(
        Path, "read_text", lambda _path: "1 (test) S " + "0 " * 18 + "7"
    )
    assert _alive(1, 7)
    assert not _alive(1, 8)

    monkeypatch.setattr(
        Path, "read_text", lambda _path: "1 (test) Z " + "0 " * 18 + "7"
    )
    assert not _alive(1, 7)

    def process_disappeared(_path: Path) -> str:
        raise ProcessLookupError

    monkeypatch.setattr(Path, "read_text", process_disappeared)
    assert not _alive(1, 7)


def test_normal_child_exit_kills_redirected_stdio_grandchild(tmp_path: Path) -> None:
    from harness.common.process import run_analyzer

    descendant_pid = tmp_path / "descendant.pid"
    descendant_code = (
        "import os,time; "
        f"open({str(descendant_pid)!r},'w').write(str(os.getpid())); "
        "time.sleep(60)"
    )
    child_code = (
        "import subprocess,sys; "
        f"p=subprocess.Popen([sys.executable,'-c',{descendant_code!r}], "
        "stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL); "
        "import time; time.sleep(.2)"
    )
    started = time.monotonic()
    result = run_analyzer([sys.executable, "-c", child_code], timeout=5)
    assert result.returncode == 0
    pid = _read_pid(descendant_pid, time.monotonic() + 2)
    state = _process_state(pid)
    assert state is None or not _alive(pid, state[1])
    assert time.monotonic() - started < 3


def test_wait_not_alive_uses_terminal_observation() -> None:
    observations = iter((False, False, True))

    def check(_pid: int, _starttime: int) -> bool:
        return next(observations)

    assert _wait_not_alive([(1, 1)], time.monotonic() + 1, check) == (False,)
    assert next(observations)


@pytest.mark.skipif(not sys.platform.startswith("linux"), reason="Linux pdeathsig")
def test_analyzer_dies_when_launcher_is_sigkilled(tmp_path: Path) -> None:
    child_pid = tmp_path / "child.pid"
    descendant_pid = tmp_path / "descendant.pid"
    descendant_code = (
        "import os,time; "
        f"open({str(descendant_pid)!r},'w').write(str(os.getpid())); "
        "time.sleep(60)"
    )
    child_code = (
        "import os,subprocess,sys,time; "
        f"open({str(child_pid)!r},'w').write(str(os.getpid())); "
        f"subprocess.Popen([sys.executable,'-c',{descendant_code!r}]); "
        "time.sleep(60)"
    )
    launcher_code = (
        "from harness.common.process import run_analyzer; "
        f"run_analyzer([{sys.executable!r}, '-c', {child_code!r}], timeout=120)"
    )
    launcher = subprocess.Popen(
        [sys.executable, "-c", launcher_code],
        env={**os.environ, "PYTHONPATH": "tools/python"},
    )
    deadline = time.monotonic() + 10
    pids = [_read_pid(child_pid, deadline), _read_pid(descendant_pid, deadline)]
    processes = []
    for pid in pids:
        state = _process_state(pid)
        assert state is not None
        processes.append((pid, state[1]))
    os.kill(launcher.pid, signal.SIGKILL)
    launcher.wait(timeout=5)
    observed = _wait_not_alive(processes, time.monotonic() + 5)
    assert not any(observed)
