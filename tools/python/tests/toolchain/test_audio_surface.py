"""Contracts for Cargo-backed audio execution and reproducible source packages."""

from __future__ import annotations

import os
from pathlib import Path
import subprocess
from types import SimpleNamespace
import zipfile

import pytest

from harness import cli
from harness.commands import audio

ROOT = Path(__file__).resolve().parents[4]


def prepare(tmp_path, monkeypatch):
    root = tmp_path / "repository with spaces"
    root.mkdir()
    config = root / "tools/rust/bof3-audio/mise.toml"
    config.parent.mkdir(parents=True)
    config.write_text('[tools]\nrust = "1.98.1"\n')
    monkeypatch.setattr(audio, "repo_layout", lambda: SimpleNamespace(root=root))
    monkeypatch.setattr(
        audio.shutil, "which", lambda name: "/tools/mise" if name == "mise" else None
    )
    calls = []
    launches = []

    def cargo(command, **kwargs):
        calls.append((command, kwargs))
        binary = root / "out/audio/release/bof3-audio"
        binary.parent.mkdir(parents=True, exist_ok=True)
        binary.touch()
        return subprocess.CompletedProcess(command, 0, "", "compiler progress")

    monkeypatch.setattr(audio.subprocess, "run", cargo)
    monkeypatch.setattr(
        audio, "exec_tool", lambda path, args: launches.append((path, args))
    )
    return root, calls, launches


@pytest.mark.parametrize("operation", audio.OPERATIONS)
def test_direct_operations_build_and_forward_without_legacy_arguments(
    tmp_path, monkeypatch, capsys, operation
):
    root, calls, launches = prepare(tmp_path, monkeypatch)
    arguments = ["--mode", "music", "--archive", "bank with spaces.EMI", "--json"]
    cli.main(["audio", operation, *arguments])
    assert launches == [
        (str(root / "out/audio/release/bof3-audio"), [operation, *arguments])
    ]
    command, options = calls[0]
    assert command[:6] == [
        "/tools/mise",
        "-C",
        str(root / "tools/rust/bof3-audio"),
        "exec",
        "--",
        "cargo",
    ]
    assert command[6:9] == ["build", "--locked", "--release"]
    assert command[9:] == [
        "--manifest-path",
        str(root / "tools/rust/bof3-audio/Cargo.toml"),
        "--target-dir",
        str(root / "out/audio"),
    ]
    assert options["cwd"] == root
    assert capsys.readouterr().out == ""


def test_build_always_checks_cargo_inputs_and_can_optionally_execute(
    tmp_path, monkeypatch, capsys
):
    root, calls, launches = prepare(tmp_path, monkeypatch)
    assert audio.build_main([]) == 0
    assert capsys.readouterr().out.strip() == str(root / "out/audio/release/bof3-audio")
    audio.build_main(["verify", "--mode", "audio", "--archive", "a.EMI"])
    assert len(calls) == 2
    assert launches[0][1] == ["verify", "--mode", "audio", "--archive", "a.EMI"]


def test_missing_cargo_does_not_launch_stale_binary(tmp_path, monkeypatch, capsys):
    _, calls, launches = prepare(tmp_path, monkeypatch)
    monkeypatch.setattr(audio.shutil, "which", lambda name: None)
    assert audio.build_main([]) == 2
    assert "mise" in capsys.readouterr().err
    assert not calls and not launches


def test_failed_build_does_not_launch_stale_binary(tmp_path, monkeypatch, capsys):
    _, _, launches = prepare(tmp_path, monkeypatch)
    audio.build_main([])
    capsys.readouterr()
    monkeypatch.setattr(
        audio.subprocess,
        "run",
        lambda command, **kwargs: subprocess.CompletedProcess(
            command, 1, "build output\n", "build error\n"
        ),
    )
    assert audio.build_main(["index", "--help"]) == 2
    captured = capsys.readouterr()
    assert captured.out == "build output\n"
    assert "build error" in captured.err and "Cargo diagnostics" in captured.err
    assert not launches


def test_missing_project_pin_does_not_use_global_toolchain(
    tmp_path, monkeypatch, capsys
):
    root, calls, launches = prepare(tmp_path, monkeypatch)
    (root / "tools/rust/bof3-audio/mise.toml").unlink()
    assert audio.build_main([]) == 2
    assert "project toolchain config" in capsys.readouterr().err
    assert not calls and not launches


def source_fixture(tmp_path):
    root = tmp_path / "repo"
    for name in audio.CRATES:
        crate = root / "tools/rust" / name
        (crate / "src").mkdir(parents=True)
        (crate / "Cargo.toml").write_text("[package]\n")
        (crate / "Cargo.lock").write_text("version = 4\n")
        (crate / "src/lib.rs").write_text("// authored source\n")
    crate = root / "tools/rust/bof3-audio"
    (crate / "mise.toml").write_text('[tools]\nrust = "1.98.1"\n')
    (crate / "README.md").write_text("source instructions\n")
    (crate / "THIRD_PARTY_LICENSES.txt").write_text("license inventory\n")
    (crate / "src/test.bin").write_bytes(b"not source")
    (crate / "target").mkdir()
    (crate / "target/artifact.rs").write_text("not authored")
    return root


def test_package_is_reproducible_includes_untracked_sources_and_excludes_media(
    tmp_path,
):
    root = source_fixture(tmp_path)
    first, second = tmp_path / "first.zip", tmp_path / "second.zip"
    audio.write_package(root, first)
    audio.write_package(root, second)
    assert first.read_bytes() == second.read_bytes()
    with zipfile.ZipFile(first) as archive:
        names = archive.namelist()
        assert len(names) == 9
        assert "bof3-audio/tools/rust/bof3-audio/mise.toml" in names
        assert "bof3-audio/tools/rust/emi-ex/src/lib.rs" in names
        assert not any("target" in name or name.endswith(".bin") for name in names)


def test_package_rejects_python_inside_audio_crate_and_preserves_destination(tmp_path):
    root = source_fixture(tmp_path)
    tests = root / "tools/rust/bof3-audio/tests"
    tests.mkdir()
    (tests / "consumer.py").write_text("# test helpers belong in Rust\n")
    output = tmp_path / "source.zip"
    output.write_bytes(b"prior package")
    with pytest.raises(ValueError, match="Python belongs in harness integration tests"):
        audio.write_package(root, output)
    assert output.read_bytes() == b"prior package"
    assert not list(tmp_path.glob(".bof3-audio-*.zip"))


def test_missing_input_and_symlink_preserve_existing_package(tmp_path):
    root = source_fixture(tmp_path)
    output = tmp_path / "source.zip"
    output.write_bytes(b"prior archive")
    lock = root / "tools/rust/emi-ex/Cargo.lock"
    lock.unlink()
    with pytest.raises(ValueError, match="missing"):
        audio.write_package(root, output)
    lock.write_text("version = 4\n")
    (root / "tools/rust/emi-ex/src/leak.rs").symlink_to(output)
    with pytest.raises(ValueError, match="symlink"):
        audio.write_package(root, output)
    assert output.read_bytes() == b"prior archive"
    assert not list(tmp_path.glob(".bof3-audio-*.zip"))


def test_failed_publication_preserves_old_archive_and_removes_staging(
    tmp_path, monkeypatch
):
    root = source_fixture(tmp_path)
    output = tmp_path / "source.zip"
    output.write_bytes(b"old archive")

    def fail(*args):
        raise OSError("publication failed")

    monkeypatch.setattr(audio.os, "replace", fail)
    with pytest.raises(OSError, match="publication failed"):
        audio.write_package(root, output)
    assert output.read_bytes() == b"old archive"
    assert not list(tmp_path.glob(".bof3-audio-*.zip"))


def test_source_package_builds_and_tests_without_proprietary_media(tmp_path):
    output = tmp_path / "source.zip"
    audio.write_package(ROOT, output)
    with zipfile.ZipFile(output) as archive:
        names = archive.namelist()
        assert not any(
            "inputs/" in name or "tools/c/" in name or "/target/" in name
            for name in names
        )
        assert "bof3-audio/tools/rust/bof3-audio/THIRD_PARTY_LICENSES.txt" in names
        archive.extractall(tmp_path / "unpacked")
    source = tmp_path / "unpacked/bof3-audio/tools/rust/bof3-audio"
    environment = dict(os.environ, MISE_STATE_DIR=str(tmp_path / "mise-state"))
    trusted = subprocess.run(
        ["mise", "trust", str(source / "mise.toml")],
        env=environment,
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert trusted.returncode == 0, trusted.stdout + trusted.stderr
    common = [
        "--locked",
        "--offline",
        "--manifest-path",
        str(source / "Cargo.toml"),
        "--target-dir",
        str(tmp_path / "cargo-target"),
    ]
    for arguments in (["build", "--release"], ["test"]):
        result = subprocess.run(
            ["mise", "-C", str(source), "exec", "--", "cargo", *arguments, *common],
            env=environment,
            capture_output=True,
            text=True,
            timeout=300,
        )
        assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run(
        [str(tmp_path / "cargo-target/release/bof3-audio"), "--help"],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert result.returncode == 0 and "--mode audio|music" in result.stdout


def test_retired_analysis_and_audio_artifacts_are_untracked_and_absent():
    retired = [
        "bin/analysis-sequence",
        "bin/psx-audio-bin",
        "tools/c/psx-audio/psx-audio",
        "psx-audio.zip",
        "tools/python/harness/commands/analysis_sequence.py",
        "tools/python/tests/test_analysis_sequence.py",
    ]
    tracked = subprocess.run(
        ["git", "ls-files", "--", *retired],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    deleted = subprocess.run(
        ["git", "ls-files", "--deleted", "--", *retired],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    assert set(tracked.stdout.splitlines()) <= set(deleted.stdout.splitlines())
    assert all(not (ROOT / path).exists() for path in retired)
