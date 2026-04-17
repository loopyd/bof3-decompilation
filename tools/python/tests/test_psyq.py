from __future__ import annotations

from pathlib import Path

from rebof3.toolchain import setup_psyq as psyq_lib
from rebof3.toolchain.setup_psyq import stage_psyq_sdk


def make_fake_psyq_tree(root: Path) -> None:
    include_dir = root / "SDK" / "INCLUDE"
    lib_dir = root / "SDK" / "LIB"
    include_dir.mkdir(parents=True)
    lib_dir.mkdir(parents=True)
    (include_dir / "LIBGPU.H").write_text("#define TEST 1\r\n", encoding="utf-8")
    (lib_dir / "LIBGPU.LIB").write_bytes(b"fake")


def test_stage_psyq_sdk_from_tree(tmp_path: Path) -> None:
    source_root = tmp_path / "psyq-source"
    dest_root = tmp_path / "psyq-staged"
    make_fake_psyq_tree(source_root)

    staged = stage_psyq_sdk(dest=dest_root, source_root=source_root)

    assert staged == dest_root.resolve()
    assert (dest_root / "include" / "LIBGPU.H").exists()
    assert (dest_root / "include" / "libgpu.h").exists()
    assert not (dest_root / "include" / "libgpu.h").is_symlink()
    assert (dest_root / "lib" / "LIBGPU.LIB").exists()
    assert (dest_root / ".gitkeep").exists()


def test_resolve_psyq_inputs_prefers_generic_env(monkeypatch) -> None:
    monkeypatch.setenv("PSYQ_SOURCE", "/tmp/generic-tree")
    monkeypatch.setenv("PSYQ40_SOURCE", "/tmp/legacy-tree")

    source_root, archive = psyq_lib._resolve_psyq_inputs(None, None)

    assert source_root == Path("/tmp/generic-tree")
    assert archive is None
