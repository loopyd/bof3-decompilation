"""Contracts for the source-built psx-audio command and package."""

from __future__ import annotations

import os
import subprocess
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]


def _run(
    *args: str, env: dict[str, str] | None = None
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=ROOT,
        capture_output=True,
        text=True,
        env=env,
        check=False,
    )


def test_audio_wrapper_configures_builds_and_executes_ignored_artifact(
    tmp_path: Path,
) -> None:
    repo = tmp_path / "repo"
    source = repo / "tools/c/psx-audio"
    (repo / "bin").mkdir(parents=True)
    source.mkdir(parents=True)
    (repo / "bin/psx-audio").write_bytes((ROOT / "bin/psx-audio").read_bytes())
    (source / "CMakeLists.txt").write_text("project(test C)\n", encoding="utf-8")

    fake_bin = tmp_path / "fake-bin"
    fake_bin.mkdir()
    log = tmp_path / "cmake.log"
    cmake = fake_bin / "cmake"
    cmake.write_text(
        "#!/bin/sh\n"
        f"printf '%s\\n' \"$*\" >> '{log}'\n"
        "printf 'cmake normal stdout\\n'\n"
        "printf 'cmake normal stderr\\n' >&2\n"
        'if [ "$1" = -S ]; then mkdir -p "$4"; : > "$4/CMakeCache.txt"; exit 0; fi\n'
        "build=$2\n"
        'mkdir -p "$build"\n'
        "cat > \"$build/bof3-audio\" <<'EOF'\n"
        "#!/bin/sh\n"
        "printf 'audio:%s\\n' \"$*\"\n"
        "EOF\n"
        'chmod +x "$build/bof3-audio"\n',
        encoding="utf-8",
    )
    cmake.chmod(0o755)
    env = os.environ.copy()
    env["PATH"] = f"{fake_bin}:{env['PATH']}"

    result = _run("sh", str(repo / "bin/psx-audio"), "--help", env=env)

    assert (result.returncode, result.stderr) == (0, "")
    assert result.stdout == f"audio:--dir {repo} --help\n"
    assert log.read_text(encoding="utf-8").splitlines() == [
        f"-S {source} -B {source / 'build'}",
        f"--build {source / 'build'} --target bof3-audio",
    ]


def test_audio_wrapper_reports_configure_failure(tmp_path: Path) -> None:
    repo = tmp_path / "repo"
    (repo / "bin").mkdir(parents=True)
    (repo / "tools/c/psx-audio").mkdir(parents=True)
    (repo / "bin/psx-audio").write_bytes((ROOT / "bin/psx-audio").read_bytes())
    fake_bin = tmp_path / "fake-bin"
    fake_bin.mkdir()
    cmake = fake_bin / "cmake"
    cmake.write_text(
        "#!/bin/sh\nprintf 'configure detail\\n'\nprintf 'configure error\\n' >&2\nexit 1\n",
        encoding="utf-8",
    )
    cmake.chmod(0o755)
    env = os.environ.copy()
    env["PATH"] = f"{fake_bin}:{env['PATH']}"

    result = _run("sh", str(repo / "bin/psx-audio"), "--help", env=env)

    assert result.returncode == 2
    assert result.stdout == "configure detail\n"
    assert result.stderr == (
        "configure error\n"
        "psx-audio setup failed: install a C compiler and zlib development files, then retry\n"
    )


def test_audio_wrapper_reports_build_failure(tmp_path: Path) -> None:
    repo = tmp_path / "repo"
    source = repo / "tools/c/psx-audio"
    (repo / "bin").mkdir(parents=True)
    source.mkdir(parents=True)
    (source / "build").mkdir()
    (source / "build/CMakeCache.txt").touch()
    (repo / "bin/psx-audio").write_bytes((ROOT / "bin/psx-audio").read_bytes())
    fake_bin = tmp_path / "fake-bin"
    fake_bin.mkdir()
    cmake = fake_bin / "cmake"
    cmake.write_text(
        "#!/bin/sh\nprintf 'build detail\\n'\nprintf 'build error\\n' >&2\nexit 1\n",
        encoding="utf-8",
    )
    cmake.chmod(0o755)
    env = os.environ.copy()
    env["PATH"] = f"{fake_bin}:{env['PATH']}"

    result = _run("sh", str(repo / "bin/psx-audio"), "--help", env=env)

    assert result.returncode == 2
    assert result.stdout == "build detail\n"
    assert result.stderr == (
        "build error\n"
        "psx-audio build failed: inspect the CMake diagnostics above, install missing development libraries, then retry\n"
    )


def test_audio_source_package_builds_all_four_native_tests(tmp_path: Path) -> None:
    archive = tmp_path / "psx-audio.zip"
    result = _run(str(ROOT / "bin/package-psx-audio"), str(archive))

    assert (result.returncode, result.stderr) == (0, "")
    with zipfile.ZipFile(archive) as package:
        names = package.namelist()
        package.extractall(tmp_path / "extracted")
    files = [name for name in names if not name.endswith("/")]
    assert "psx-audio/CMakeLists.txt" in files
    assert all(
        name == "psx-audio/CMakeLists.txt" or name.endswith((".c", ".h"))
        for name in files
    )

    source = tmp_path / "extracted/psx-audio"
    build = tmp_path / "package-build"
    configured = _run("cmake", "-S", str(source), "-B", str(build))
    built = _run("cmake", "--build", str(build))
    tested = _run("ctest", "--test-dir", str(build), "--output-on-failure")

    assert configured.returncode == 0, configured.stdout + configured.stderr
    assert built.returncode == 0, built.stdout + built.stderr
    assert tested.returncode == 0, tested.stdout + tested.stderr
    assert "100% tests passed, 0 tests failed out of 4" in tested.stdout


def test_retired_analysis_and_audio_artifacts_are_untracked_and_absent() -> None:
    retired = [
        "bin/analysis-sequence",
        "bin/psx-audio-bin",
        "tools/c/psx-audio/psx-audio",
        "psx-audio.zip",
        "tools/python/harness/commands/analysis_sequence.py",
        "tools/python/tests/test_analysis_sequence.py",
    ]
    tracked = _run("git", "ls-files", "--", *retired)
    deleted = _run("git", "ls-files", "--deleted", "--", *retired)

    assert (tracked.returncode, deleted.returncode) == (0, 0)
    assert tracked.stderr == deleted.stderr == ""
    assert set(tracked.stdout.splitlines()) <= set(deleted.stdout.splitlines())
    assert all(not (ROOT / path).exists() for path in retired)
