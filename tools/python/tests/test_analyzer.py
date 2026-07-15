"""Tests for harness.analyzer subprocess-based stateless adapters."""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness import analyzer
from harness.analyzer import EngineIdentity, doctor, find_best_engine, find_engine


class _FakeResult:
    def __init__(
        self,
        stdout: str = "",
        stderr: str = "",
        returncode: int = 0,
    ):
        self.stdout = stdout
        self.stderr = stderr
        self.returncode = returncode


def _extract_commands(argv: list[str]) -> list[str]:
    commands: list[str] = []
    i = 0
    while i < len(argv):
        if argv[i] == "-c" and i + 1 < len(argv):
            commands.append(argv[i + 1])
            i += 2
        else:
            i += 1
    return commands


# ---------------------------------------------------------------------------
# Engine probing
# ---------------------------------------------------------------------------


def test_find_engine_missing_raises(monkeypatch: pytest.MonkeyPatch) -> None:
    monkeypatch.setattr("shutil.which", lambda name: None)
    with pytest.raises(FileNotFoundError, match="rizin not found"):
        find_engine("rizin")


def test_find_engine_rizin_probes_capabilities(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        flag = cmd[1] if len(cmd) > 1 else ""
        if flag == "-V":
            return _FakeResult(stdout="rizin 0.8.2\n")
        if flag == "-q0":
            commands = _extract_commands(cmd)
            if commands == ["e asm.arch", "e asm.bits", "e cfg.bigendian"]:
                return _FakeResult(stdout="mips\n32\nfalse\n")
            if commands == ["aflj", "axlj"]:
                return _FakeResult(stdout="[]\n[]\n")
            if commands == ["P?"]:
                return _FakeResult(stdout="Project management commands\n")
            if commands == ["pdg?"]:
                return _FakeResult(stdout="decompiler\n")
        return _FakeResult()

    monkeypatch.setattr("shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(subprocess, "run", fake_run)

    identity = find_engine("rizin")
    assert identity.name == "rizin"
    assert identity.version == "rizin 0.8.2"
    assert identity.capabilities["mips32_little_endian"] is True
    assert identity.capabilities["json"] is True
    assert identity.capabilities["projects"] is True
    assert identity.capabilities["decompiler"] is True


def test_find_engine_r2_probes_capabilities(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        flag = cmd[1] if len(cmd) > 1 else ""
        if flag == "-v":
            return _FakeResult(stdout="radare2 6.1.4\n")
        if flag == "-q0":
            commands = _extract_commands(cmd)
            if commands == ["e asm.arch", "e asm.bits", "e cfg.bigendian"]:
                return _FakeResult(stdout="mips\n32\nfalse\n")
            if commands == ["aflj", "axlj"]:
                return _FakeResult(stdout="[]\n[]\n")
            if commands == ["P?"]:
                return _FakeResult(stdout="Project management commands\n")
            if commands == ["pdg?"]:
                return _FakeResult(stdout="decompiler\n")
        return _FakeResult()

    monkeypatch.setattr("shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(subprocess, "run", fake_run)

    identity = find_engine("r2")
    assert identity.name == "r2"
    assert identity.version == "radare2 6.1.4"
    assert identity.capabilities["mips32_little_endian"] is True


def test_find_engine_empty_version_raises(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        return _FakeResult(stdout="", stderr="")

    monkeypatch.setattr("shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(subprocess, "run", fake_run)

    with pytest.raises(RuntimeError, match="returned empty output"):
        find_engine("rizin")


def test_find_engine_bad_json_capability(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        flag = cmd[1] if len(cmd) > 1 else ""
        if flag == "-V":
            return _FakeResult(stdout="rizin 0.8.2\n")
        if flag == "-q0":
            commands = _extract_commands(cmd)
            if commands == ["e asm.arch", "e asm.bits", "e cfg.bigendian"]:
                return _FakeResult(stdout="mips\n32\nfalse\n")
            if commands == ["aflj", "axlj"]:
                return _FakeResult(stdout="not json\nalso bad\n")
        return _FakeResult()

    monkeypatch.setattr("shutil.which", lambda name: f"/usr/bin/{name}")
    monkeypatch.setattr(subprocess, "run", fake_run)

    with pytest.raises(RuntimeError, match="required capabilities: json"):
        find_engine("rizin")


# ---------------------------------------------------------------------------
# Capability selection
# ---------------------------------------------------------------------------


def test_find_best_engine_auto_selects_available_engine(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_find(name: str) -> EngineIdentity:
        if name == "rizin":
            return EngineIdentity(
                name="rizin",
                executable=Path("/usr/bin/rizin"),
                version="rizin 0.8.2",
                capabilities={"mips32_little_endian": True, "json": True},
            )
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    best = find_best_engine()
    assert best.name == "rizin"


def test_find_best_engine_falls_back_to_r2(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    calls: list[str] = []

    def fake_find(name: str) -> EngineIdentity:
        calls.append(name)
        if name == "rizin":
            raise FileNotFoundError("rizin not found")
        if name == "r2":
            return EngineIdentity(
                name="r2",
                executable=Path("/usr/bin/r2"),
                version="radare2 6.1.4",
                capabilities={
                    "mips32_little_endian": True,
                    "json": True,
                },
            )
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    best = find_best_engine()
    assert best.name == "r2"
    assert calls == ["rizin", "r2"]


def test_find_best_engine_honors_explicit_engine(monkeypatch: pytest.MonkeyPatch) -> None:
    calls: list[str] = []

    def fake_find(name: str) -> EngineIdentity:
        calls.append(name)
        return EngineIdentity(
            name=name,
            executable=Path(f"/usr/bin/{name}"),
            version="test",
            capabilities={"mips32_little_endian": True, "json": True},
        )

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    assert find_best_engine("r2").name == "r2"
    assert calls == ["r2"]


def test_find_best_engine_rejects_unknown_engine() -> None:
    with pytest.raises(ValueError, match="must be one of"):
        find_best_engine("ghidra")


def test_find_best_engine_fails_when_neither_available(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_find(name: str) -> EngineIdentity:
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    with pytest.raises(FileNotFoundError, match="neither rizin nor r2"):
        find_best_engine()


# ---------------------------------------------------------------------------
# Timeout and malformed JSON
# ---------------------------------------------------------------------------


def test_query_project_timeout(monkeypatch: pytest.MonkeyPatch) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        timeout = kwargs.get("timeout")
        if timeout is not None:
            raise subprocess.TimeoutExpired(cmd=cmd, timeout=timeout)
        return _FakeResult()

    monkeypatch.setattr(subprocess, "run", fake_run)

    engine = EngineIdentity(
        name="rizin",
        executable=Path("/usr/bin/rizin"),
        version="rizin 0.8.2",
        capabilities={"json": True},
    )
    binary_path = Path("out/test.bin")
    load_address = 0x801D0C00

    with pytest.raises(subprocess.TimeoutExpired):
        analyzer.query_project(engine, binary_path, load_address, "functions", timeout=1)


def test_query_project_malformed_json_returns_empty(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        return _FakeResult(stdout="not json\n")

    monkeypatch.setattr(subprocess, "run", fake_run)

    engine = EngineIdentity(
        name="rizin",
        executable=Path("/usr/bin/rizin"),
        version="rizin 0.8.2",
        capabilities={"json": True},
    )
    binary_path = Path("out/test.bin")
    load_address = 0x801D0C00

    result = analyzer.query_project(engine, binary_path, load_address, "functions", timeout=1)
    assert result == []


def test_query_project_analyzes_before_query(monkeypatch: pytest.MonkeyPatch) -> None:
    commands_seen: list[str] = []

    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        commands_seen.extend(_extract_commands(cmd))
        return _FakeResult(stdout="[]\n")

    monkeypatch.setattr(subprocess, "run", fake_run)

    engine = EngineIdentity(
        name="rizin",
        executable=Path("/usr/bin/rizin"),
        version="rizin 0.8.2",
        capabilities={"mips32_little_endian": True, "json": True},
    )
    analyzer.query_project(engine, Path("out/test.bin"), 0x801D0C00, "functions")
    assert commands_seen == ["aa", "aflj"]


def test_query_project_seeds_reviewed_functions(monkeypatch: pytest.MonkeyPatch) -> None:
    commands_seen: list[str] = []

    def fake_run(cmd: list[str], **kwargs: object) -> _FakeResult:
        commands_seen.extend(_extract_commands(cmd))
        return _FakeResult(stdout="[]\n")

    monkeypatch.setattr(subprocess, "run", fake_run)
    engine = EngineIdentity(
        name="rizin",
        executable=Path("/usr/bin/rizin"),
        version="rizin 0.8.2",
        capabilities={"mips32_little_endian": True, "json": True},
    )

    analyzer.query_project(
        engine,
        Path("out/test.bin"),
        0x801D0C00,
        "functions",
        setup_commands=["af @ 0x801D0C00"],
    )

    assert commands_seen == ["aa", "af @ 0x801D0C00", "aflj"]


# ---------------------------------------------------------------------------
# Doctor
# ---------------------------------------------------------------------------


def test_doctor_reports_rizin(monkeypatch: pytest.MonkeyPatch) -> None:
    def fake_find(name: str) -> EngineIdentity:
        if name == "rizin":
            return EngineIdentity(
                name="rizin",
                executable=Path("/usr/bin/rizin"),
                version="rizin 0.8.2",
                capabilities={
                    "mips32_little_endian": True,
                    "json": True,
                },
            )
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    result = doctor()
    assert result["engine"] == "rizin"
    assert result["available"] is True


def test_doctor_reports_r2_fallback(monkeypatch: pytest.MonkeyPatch) -> None:
    def fake_find(name: str) -> EngineIdentity:
        if name == "rizin":
            raise FileNotFoundError("rizin not found")
        if name == "r2":
            return EngineIdentity(
                name="r2",
                executable=Path("/usr/bin/r2"),
                version="radare2 6.1.4",
                capabilities={
                    "mips32_little_endian": True,
                    "json": True,
                },
            )
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    result = doctor()
    assert result["engine"] == "r2"
    assert result["available"] is True


def test_doctor_reports_unavailable(monkeypatch: pytest.MonkeyPatch) -> None:
    def fake_find(name: str) -> EngineIdentity:
        raise FileNotFoundError(f"{name} not found")

    monkeypatch.setattr(analyzer, "find_engine", fake_find)
    result = doctor()
    assert result["engine"] is None
    assert result["available"] is False
    assert "not found" in result["error"]
