from __future__ import annotations

from pathlib import Path
import subprocess

import pytest

from harness.workflows.permuter import (
    _original_target_assembly,
    _repair_psyq_register_parameters,
    run_permuter,
)


def test_repairs_psyq_fp_parameter_without_touching_asm_strings() -> None:
    source = 'CdlFILE *CdSearchFile(CdlFILE *$30, char *name);\nasm("mtc2 $12,$30");\n'

    repaired = _repair_psyq_register_parameters(source)

    assert "CdlFILE *fp" in repaired
    assert 'asm("mtc2 $12,$30")' in repaired


def test_original_target_assembly_uses_authoritative_little_endian_words() -> None:
    assembly = _original_target_assembly("func_801625e4", bytes.fromhex("e8ffbd27"))

    assert ".globl func_801625e4" in assembly
    assert ".word 0x27bdffe8" in assembly


def test_original_target_assembly_rejects_partial_instruction() -> None:
    with pytest.raises(ValueError, match="word-aligned"):
        _original_target_assembly("func_801625e4", b"abc")


def _permuter_fixture(tmp_path: Path) -> tuple[Path, dict[str, str]]:
    bundle = tmp_path / "out" / "matching" / "target" / "func_801625e4" / "permuter"
    bundle.mkdir(parents=True)
    (bundle / "base.c").write_text("void func_801625e4(void) {}\n", encoding="utf-8")
    (bundle / "target.o").write_bytes(b"target")
    (bundle / "compile.sh").write_text("#!/bin/sh\n", encoding="utf-8")
    (bundle / "compile.sh").chmod(0o755)
    tool = tmp_path / "third_party" / "decomp-permuter" / "permuter.py"
    tool.parent.mkdir(parents=True)
    tool.write_text("# fixture\n", encoding="utf-8")
    return bundle, {
        "bundle": str(bundle.relative_to(tmp_path)),
        "function": "target@801625e4",
    }


def test_run_permuter_preflights_and_bounds_deterministic_iterations(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    bundle, metadata = _permuter_fixture(tmp_path)
    commands: list[list[str]] = []

    def fake_run(command: list[str], **_: object) -> subprocess.CompletedProcess[str]:
        commands.append(command)
        if command[0].endswith("compile.sh"):
            Path(command[-1]).write_bytes(b"object")
        else:
            iteration = len(commands) - 1
            output = bundle / f"output-{10 - iteration}-{iteration}"
            output.mkdir()
            (output / "source.c").write_text(f"/* {iteration} */\n", encoding="utf-8")
            (output / "score.txt").write_text(f"{10 - iteration}\n", encoding="utf-8")
        return subprocess.CompletedProcess(command, 0, "ok\n", "")

    monkeypatch.setattr(subprocess, "run", fake_run)

    result = run_permuter(tmp_path, metadata, jobs=1, iterations=3, timeout=10, seed=40)

    assert [command[-1] for command in commands[1:]] == ["40", "41", "42"]
    assert result["iterations_completed"] == 3
    assert result["status"] == "improved"
    assert result["best"]["score"] == 7
    assert not (bundle / "base-preflight.o").exists()


def test_run_permuter_reports_timeout_as_failure(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    bundle, metadata = _permuter_fixture(tmp_path)

    def fake_run(command: list[str], **_: object) -> subprocess.CompletedProcess[str]:
        if command[0].endswith("compile.sh"):
            Path(command[-1]).write_bytes(b"object")
            return subprocess.CompletedProcess(command, 0, "", "")
        raise subprocess.TimeoutExpired(command, 1)

    monkeypatch.setattr(subprocess, "run", fake_run)

    result = run_permuter(tmp_path, metadata, iterations=1, timeout=1)

    assert result["status"] == "failed"
    assert result["failure_count"] == 1
    assert result["iterations_completed"] == 0
    assert (bundle / "result.json").is_file()
