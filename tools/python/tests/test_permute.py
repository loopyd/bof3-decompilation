"""Characterization tests for the permute coordinator.

These tests lock current coordinator behavior before any extraction refactor.
They use tmp_path roots and mocked process calls; they never launch the real
decomp-permuter.
"""

from __future__ import annotations

import subprocess
import sys
import pytest

from harness.commands import permute
from harness.toolchain.permuter import DecompPermuterToolchain


ROOT = Path(__file__).resolve().parents[3]


def _stub_tools(root: Path) -> None:
    (root / "tools").mkdir(parents=True, exist_ok=True)
    (root / "tools" / "prep-permuter.py").touch()


# ---------------------------------------------------------------------------
# Preparer
# ---------------------------------------------------------------------------


def test_preparer_creates_runnable_workspace(tmp_path: Path) -> None:
    source = tmp_path / "func_test.c"
    source.write_text("typedef int s32;\ns32 func_test(void) { return 1; }\n")
    (tmp_path / "target.s").write_text(".text\nglabel func_test\n    jr $ra\n     li $v0, 1\n")

    subprocess.run(
        [
            sys.executable,
            str(ROOT / "tools" / "prep-permuter.py"),
            str(source),
            "func_test",
            str(tmp_path),
        ],
        check=True,
    )
    subprocess.run(
        [str(tmp_path / "compile.sh"), str(tmp_path / "base.c"), "-o", str(tmp_path / "base.o")],
        check=True,
    )

    for name in ("base.c", "compile.sh", "settings.toml", "target.o"):
        assert (tmp_path / name).is_file()


# ---------------------------------------------------------------------------
# Parser
# ---------------------------------------------------------------------------


def test_example_returns_0() -> None:
    assert permute.main(["--example"]) == 0


def test_missing_source_returns_2(capsys: pytest.CaptureFixture) -> None:
    assert permute.main([]) == 2
    assert "error:" in capsys.readouterr().err


def test_nonexistent_source_returns_2(
    tmp_path: Path, capsys: pytest.CaptureFixture
) -> None:
    rc = permute.main(["--root", str(tmp_path), str(tmp_path / "missing.c")])
    assert rc == 2
    assert "error:" in capsys.readouterr().err


# ---------------------------------------------------------------------------
# function_name
# ---------------------------------------------------------------------------


def test_function_name_from_stem() -> None:
    assert permute.function_name(Path("/src/x/func_801CE758.c"), None) == "func_801CE758"


def test_function_name_explicit() -> None:
    assert permute.function_name(Path("src/x.c"), "custom") == "custom"


def test_function_name_invalid_raises() -> None:
    with pytest.raises(ValueError, match="invalid function name"):
        permute.function_name(Path("x.c"), "bad-name!")


# ---------------------------------------------------------------------------
# require_inside_root and default_directory
# ---------------------------------------------------------------------------


def test_require_inside_root_passes(tmp_path: Path) -> None:
    inside = tmp_path / "src" / "valid.c"
    inside.parent.mkdir(parents=True)
    inside.touch()
    permute.require_inside_root(inside, "test", tmp_path)


def test_require_inside_root_raises(tmp_path: Path) -> None:
    outside = tmp_path.parent / "outside.c"
    with pytest.raises(ValueError, match="must be inside"):
        permute.require_inside_root(outside, "test", tmp_path)


def test_default_directory_includes_src(tmp_path: Path) -> None:
    source = tmp_path / "src" / "exe" / "logo" / "func_801CE758.c"
    source.parent.mkdir(parents=True)
    source.touch()
    result = permute.default_directory(source, tmp_path)
    expected = (tmp_path / "out" / "permuter" / "src" / "exe" / "logo" / "func_801CE758").resolve()
    assert result == expected


# ---------------------------------------------------------------------------
# permuter_arguments
# ---------------------------------------------------------------------------


def test_permuter_arguments_defaults() -> None:
    args = permute.build_parser().parse_args(["--root", "/fake"])
    opts = permute.permuter_arguments(args)
    assert "--best-only" in opts
    assert "--quiet" not in opts


def test_permuter_arguments_quiet() -> None:
    args = permute.build_parser().parse_args(["--root", "/fake", "--quiet"])
    opts = permute.permuter_arguments(args)
    assert "--quiet" in opts


def test_permuter_arguments_algorithm() -> None:
    args = permute.build_parser().parse_args(
        ["--root", "/fake", "--algorithm", "levenshtein"]
    )
    opts = permute.permuter_arguments(args)
    assert "--algorithm" in opts
    assert "levenshtein" in opts


def test_permuter_arguments_jobs() -> None:
    args = permute.build_parser().parse_args(["--root", "/fake", "-j", "4"])
    opts = permute.permuter_arguments(args)
    assert "-j" in opts
    assert "4" in opts


# ---------------------------------------------------------------------------
# run argument validation
# ---------------------------------------------------------------------------


def test_run_requires_preparer(tmp_path: Path) -> None:
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    ns = permute.build_parser().parse_args(["--root", str(tmp_path), str(src)])
    with pytest.raises(FileNotFoundError, match="decomp-permuter workflow"):
        permute.run(ns)


def test_run_rejects_negative_jobs(tmp_path: Path) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "-j", "-1"]
    )
    with pytest.raises(ValueError, match="--jobs must not be negative"):
        permute.run(ns)


def test_run_rejects_zero_time_limit(tmp_path: Path) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "--time-limit", "0"]
    )
    with pytest.raises(ValueError, match="--time-limit must be positive"):
        permute.run(ns)


def test_run_rejects_prepare_only_with_prepared(tmp_path: Path) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "--prepare-only", "--prepared"]
    )
    with pytest.raises(ValueError, match="--prepare-only cannot be combined with --prepared"):
        permute.run(ns)


# ---------------------------------------------------------------------------
# run — prepared workspace validation
# ---------------------------------------------------------------------------


def test_run_prepared_missing_files_raises(tmp_path: Path) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    with pytest.raises(FileNotFoundError, match="prepared workspace is missing"):
        permute.run(ns)


def test_run_prepared_accepts_complete_workspace(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    monkeypatch.setattr(subprocess, "run", lambda command, **kw: subprocess.CompletedProcess(command, 0))
    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    rc = permute.run(ns)
    assert rc == 0


# ---------------------------------------------------------------------------
# run — prepare-only flow
# ---------------------------------------------------------------------------


def test_run_prepare_only_propagates_preparer_nonzero(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    asm = tmp_path / "out" / "splat" / "exe" / "x" / "asm" / "f.s"
    asm.parent.mkdir(parents=True)
    asm.write_text("glabel f\n")

    monkeypatch.setattr(subprocess, "run", lambda command, **kw: subprocess.CompletedProcess(command, 7))
    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "--prepare-only"]
    )
    rc = permute.run(ns)
    assert rc == 7


def test_run_prepare_only_returns_zero(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    asm = tmp_path / "out" / "splat" / "exe" / "x" / "asm" / "f.s"
    asm.parent.mkdir(parents=True)
    asm.write_text("glabel f\n")

    monkeypatch.setattr(subprocess, "run", lambda command, **kw: subprocess.CompletedProcess(command, 0))
    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "--prepare-only"]
    )
    rc = permute.run(ns)
    assert rc == 0


# ---------------------------------------------------------------------------
# run — upstream permuter invocation via toolchain
# ---------------------------------------------------------------------------


def test_run_delegates_permuter_through_toolchain(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    calls: list[tuple[Path | None, list[str], dict[str, Any]]] = []

    def fake_execute(
        self: object, args: list[str], **kwargs: Any
    ) -> subprocess.CompletedProcess[str]:
        root = getattr(self, "root", None)
        calls.append((root, args, kwargs))
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)
    monkeypatch.setattr(DecompPermuterToolchain, "execute", fake_execute)

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    rc = permute.run(ns)
    assert rc == 0
    assert len(calls) == 1
    call_root, call_args, call_kwargs = calls[0]
    assert call_root == tmp_path
    assert "-u" not in call_args  # interpreter flag, now owned by toolchain
    assert str(workspace) == call_args[-1]
    assert call_kwargs.get("timeout") is None


def test_run_propagates_permuter_exit_code(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)
    monkeypatch.setattr(
        DecompPermuterToolchain,
        "execute",
        lambda self, args, **kw: subprocess.CompletedProcess(args, 5, stdout="", stderr=""),
    )

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    rc = permute.run(ns)
    assert rc == 5


# ---------------------------------------------------------------------------
# run — timeout handling
# ---------------------------------------------------------------------------


def test_run_timeout_returns_zero(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)
    monkeypatch.setattr(
        DecompPermuterToolchain,
        "execute",
        lambda self, args, **kw: (_ for _ in ()).throw(
            subprocess.TimeoutExpired(args, timeout=5)
        ),
    )

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace), "--time-limit", "5"]
    )
    rc = permute.run(ns)
    assert rc == 0


# ---------------------------------------------------------------------------
# run — lock behavior
# ---------------------------------------------------------------------------


def test_run_acquires_and_releases_lock(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    lock_path = workspace / ".coordinator.lock"
    acquired: list[int] = []

    def track_flock(fd: int, op: int) -> None:
        if op == permute.fcntl.LOCK_EX | permute.fcntl.LOCK_NB:
            acquired.append(fd)

    monkeypatch.setattr(permute.fcntl, "flock", track_flock)
    monkeypatch.setattr(
        DecompPermuterToolchain,
        "execute",
        lambda self, args, **kw: subprocess.CompletedProcess(args, 0, stdout="", stderr=""),
    )

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    rc = permute.run(ns)
    assert rc == 0
    assert len(acquired) == 1
    assert not lock_path.is_file()


def test_run_concurrent_lock_raises(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "x" / "f.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    def blocking_flock(fd: int, op: int) -> None:
        raise BlockingIOError(11, "Resource temporarily unavailable")

    monkeypatch.setattr(permute.fcntl, "flock", blocking_flock)

    ns = permute.build_parser().parse_args(
        ["--root", str(tmp_path), str(src), "f", "--prepared", str(workspace)]
    )
    with pytest.raises(RuntimeError, match="another decomp-permuter coordinator"):
        permute.run(ns)


# ---------------------------------------------------------------------------
# main — TARGET@0xADDRESS resolution
# ---------------------------------------------------------------------------


def test_main_resolves_target_address(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """TARGET@0xADDRESS is resolved through lift.resolve_function and passed to run."""
    _stub_tools(tmp_path)
    src = tmp_path / "src" / "exe" / "logo" / "func_801CE758.c"
    src.parent.mkdir(parents=True)
    src.touch()
    workspace = tmp_path / "workspace"
    workspace.mkdir()
    for name in ("base.c", "target.o", "compile.sh", "settings.toml"):
        (workspace / name).touch()

    from types import SimpleNamespace
    mock_id = SimpleNamespace(
        address=0x801CE758, target=SimpleNamespace(value="exe/logo")
    )

    monkeypatch.setattr(
        "harness.commands.lift.resolve_function",
        lambda raw: (mock_id, mock_id.target, src),
    )
    monkeypatch.setattr(permute.fcntl, "flock", lambda fd, op: None)
    monkeypatch.setattr(
        DecompPermuterToolchain,
        "execute",
        lambda self, args, **kw: subprocess.CompletedProcess(args, 0, stdout="", stderr=""),
    )

    rc = permute.main(
        [
            "--root",
            str(tmp_path),
            "exe/logo@0x801CE758",
            "f",
            "--prepared",
            str(workspace),
        ]
    )
    assert rc == 0
