"""Focused tests for bin/splat delegation through SplatToolchain."""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness.commands import splat
from harness.toolchain.splat import SplatToolchain


def _setup_target(tmp_path: Path) -> Path:
    """Create a minimal exe/logo target and venv so the command can parse."""
    root = tmp_path
    target_dir = root / "config" / "targets" / "exe" / "logo"
    target_dir.mkdir(parents=True)
    (target_dir / "manifest.toml").write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\n'
        'kind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n'
        "load_address = 0x80100000\n",
        encoding="utf-8",
    )
    (root / "out/binaries/exe/logo.bin").parent.mkdir(parents=True)
    (root / "out/binaries/exe/logo.bin").write_bytes(b"\x00" * 0x200000)
    # Create .venv/bin/splat so SplatToolchain.executable resolves
    splat_exe = root / ".venv" / "bin" / "splat"
    splat_exe.parent.mkdir(parents=True)
    splat_exe.touch()
    return root


def test_missing_executable_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    root.joinpath(".venv/bin/splat").unlink()
    with pytest.raises(FileNotFoundError, match="missing Splat executable"):
        splat.run(
            splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
        )


def test_non_verbose_captures_output(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 0
    assert len(calls) == 1
    args_passed, kwargs = calls[0]
    assert kwargs.get("capture_output") is True
    assert kwargs.get("text") is True


@pytest.mark.parametrize(
    "owner_text",
    [
        "/* @source 0x80100000 @behavior loads entry */\n",
        "void loadEntry(void) {}\n",
        "/* @source 0x80100004 @behavior wrong address */\n",
    ],
)
def test_generated_root_stub_projected_or_kept(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, owner_text: str
) -> None:
    """Fresh Splat never deletes src files: a Splat-regenerated legacy stub is
    preserved in the ignored projection when its explicit @source owner
    matches, and authored/foreign files stay untouched in src/."""

    root = _setup_target(tmp_path)
    source_dir = root / "src/exe/logo"
    owner = source_dir / "io/loadEntry.c"
    owner.parent.mkdir(parents=True)
    owner.write_text(owner_text)
    stub = source_dir / "loadEntry.c"
    splat_path = root / "config/targets/exe/logo/splat.yaml"
    splat_path.write_text(
        "name: logo\nsegments:\n- name: main\n  type: code\n  start: 0\n"
        "  vram: 0x80100000\n  subsegments:\n"
        "  - - 0\n    - c\n    - loadEntry\n"
        "    - '@source: src/exe/logo/io/loadEntry.c'\n"
    )

    def fake_execute(self, args, **kwargs):
        stub.write_text('#include "common.h"\nINCLUDE_ASM("x", loadEntry);\n')
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    assert splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    ) == 0
    projected = root / "out/splat/exe/logo/source-view/loadEntry.c"
    if owner_text.startswith("/* @source 0x80100000"):
        # Matching owner: the generated stub is preserved in the projection.
        assert owner.is_file()
        assert not stub.exists()
        assert "INCLUDE_ASM" in projected.read_text()
    else:
        # No matching metadata: nothing under src/ may change.
        assert owner.is_file()
        assert stub.is_file()
        assert not projected.exists()


def test_repeat_run_refreshes_projection_and_removes_stub(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Repeated Splat with an existing projection is idempotent: every freshly
    regenerated metadata-free root stub is projected (atomic refresh, never a
    skip) and removed from src/ so Splat then build passes every time."""

    root = _setup_target(tmp_path)
    source_dir = root / "src/exe/logo"
    owner = source_dir / "io/loadEntry.c"
    owner.parent.mkdir(parents=True)
    owner.write_text("/* @source 0x80100000 @behavior loads entry */\n")
    stub = source_dir / "loadEntry.c"
    splat_path = root / "config/targets/exe/logo/splat.yaml"
    splat_path.write_text(
        "name: logo\nsegments:\n- name: main\n  type: code\n  start: 0\n"
        "  vram: 0x80100000\n  subsegments:\n"
        "  - - 0\n    - c\n    - loadEntry\n"
        "    - '@source: src/exe/logo/io/loadEntry.c'\n"
    )
    projected = root / "out/splat/exe/logo/source-view/loadEntry.c"
    projected.parent.mkdir(parents=True)
    projected.write_text("stale projection bytes")

    def fake_execute(self, args, **kwargs):
        stub.write_text('#include "common.h"\nINCLUDE_ASM("x", loadEntry);\n')
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    parser = splat.build_parser()
    for _ in range(2):
        assert splat.run(parser.parse_args(["--root", str(root), "exe/logo"])) == 0
        assert owner.is_file()
        assert not stub.exists()
        assert "INCLUDE_ASM" in projected.read_text()


def test_out_of_root_owner_renamed_basename_stub_projected(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """Splat stubs are keyed by the Splat boundary name, never the authored
    destination basename: a collision-renamed out-of-root owner
    (``advancePanelXTo320_game00_801996FC.c`` under boundary
    ``advancePanelXTo320``) still gets its regenerated root stub projected and
    removed from src/."""

    root = _setup_target(tmp_path)
    source_dir = root / "src/exe/logo"
    source_dir.mkdir(parents=True, exist_ok=True)
    owner = root / "src/bof3/ui/advancePanelXTo320_game00_801996FC.c"
    owner.parent.mkdir(parents=True)
    owner.write_text(
        "/* @source 0x801996FC @behavior advances panel x */\n", encoding="utf-8"
    )
    stub = source_dir / "advancePanelXTo320.c"
    splat_path = root / "config/targets/exe/logo/splat.yaml"
    splat_path.write_text(
        "name: logo\nsegments:\n- name: main\n  type: code\n  start: 0\n"
        "  vram: 0x801996FC\n  subsegments:\n"
        "  - - 0\n    - c\n    - advancePanelXTo320\n"
        "    - '@source: src/bof3/ui/advancePanelXTo320_game00_801996FC.c'\n"
        "    - '@behavior: advances panel x'\n"
    )
    projected = root / "out/splat/exe/logo/source-view/advancePanelXTo320_game00_801996FC.c"
    projected.parent.mkdir(parents=True)
    projected.write_text("stale projection bytes")

    def fake_execute(self, args, **kwargs):
        stub.write_text('#include "common.h"\nINCLUDE_ASM("x", advancePanelXTo320);\n')
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    parser = splat.build_parser()
    for _ in range(2):
        assert splat.run(parser.parse_args(["--root", str(root), "exe/logo"])) == 0
        assert owner.is_file()
        assert not stub.exists()
        assert "INCLUDE_ASM" in projected.read_text()


def test_pre_run_refuses_authored_source_at_legacy_stub_path(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """A metadata-tagged (authored) file at a legacy path Splat could write
    must refuse the run before Splat ever executes."""

    root = _setup_target(tmp_path)
    source_dir = root / "src/exe/logo"
    owner = source_dir / "io/loadEntry.c"
    owner.parent.mkdir(parents=True)
    owner.write_text("/* @source 0x80100000 @behavior loads entry */\n")
    stub = source_dir / "loadEntry.c"
    stub.write_text("/* @source 0x80100000 @behavior authored duplicate */\n")
    splat_path = root / "config/targets/exe/logo/splat.yaml"
    splat_path.write_text(
        "name: logo\nsegments:\n- name: main\n  type: code\n  start: 0\n"
        "  vram: 0x80100000\n  subsegments:\n"
        "  - - 0\n    - c\n    - loadEntry\n"
        "    - '@source: src/exe/logo/io/loadEntry.c'\n"
    )
    calls: list = []

    def fake_execute(self, args, **kwargs):
        calls.append(args)
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    with pytest.raises(ValueError, match="refusing Splat"):
        splat.run(
            splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
        )
    assert calls == []
    assert stub.is_file()
    assert owner.is_file()


def test_verbose_streams_output(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0)

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(
            ["--root", str(root), "exe/logo", "--verbose"]
        )
    )
    assert rc == 0
    assert len(calls) == 1
    _, kwargs = calls[0]
    assert kwargs.get("capture_output") is False
    assert kwargs.get("text") is False


def test_non_verbose_failure_prints_to_stderr(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    root = _setup_target(tmp_path)

    def fake_execute(self, args, **kwargs):
        return subprocess.CompletedProcess(
            args, 7, stdout="stdout-msg", stderr="stderr-msg"
        )

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 7
    captured = capsys.readouterr()
    assert captured.out == ""
    assert "stdout-msg" in captured.err
    assert "stderr-msg" in captured.err


def test_non_verbose_success_prints_ok(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    root = _setup_target(tmp_path)

    def fake_execute(self, args, **kwargs):
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 0
    captured = capsys.readouterr()
    assert "splat OK" in captured.out
    assert captured.err == ""


def test_splat_path_in_arguments(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert len(calls) == 1
    args_passed = calls[0][0]
    assert "split" in args_passed
    assert "--make-full-disasm-for-code" in args_passed
    assert any("splat.yaml" in a for a in args_passed)
