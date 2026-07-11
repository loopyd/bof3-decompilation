from __future__ import annotations

from pathlib import Path
import zipfile

from rebof3.toolchain import setup_psyq as psyq_lib
from rebof3.toolchain.setup_psyq import import_psyq_sdk, stage_psyq_sdk


def make_fake_psyq_tree(root: Path) -> None:
    include_dir = root / "SDK" / "INCLUDE"
    lib_dir = root / "SDK" / "LIB"
    include_dir.mkdir(parents=True)
    lib_dir.mkdir(parents=True)
    (include_dir / "LIBGPU.H").write_text("#define TEST 1\r\n", encoding="utf-8")
    (lib_dir / "LIBGPU.LIB").write_bytes(b"fake")


def test_stage_psyq_sdk_from_tree(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(psyq_lib, "REPO_ROOT", tmp_path)
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


def test_import_psyq_sdk_stages_original_archive(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(psyq_lib, "REPO_ROOT", tmp_path)
    archive_path = tmp_path / "psyq47.zip"
    private_root = tmp_path / "private-assets"
    dest_root = tmp_path / "psyq-staged"
    with zipfile.ZipFile(archive_path, "w") as archive:
        archive.writestr("SDK/INCLUDE/LIBGPU.H", "#define TEST 1\r\n")
        archive.writestr("SDK/LIB/LIBGPU.LIB", b"fake")

    staged = import_psyq_sdk(
        dest=dest_root,
        archive=archive_path,
        private_assets_root=private_root,
        archive_url=None,
    )

    assert staged == dest_root.resolve()
    assert (private_root / "psyq" / "4.7" / "source-media" / archive_path.name).exists()
    assert (dest_root / "include" / "libgpu.h").exists()


def test_find_psyq_source_rejects_explicit_paths_outside_repo(tmp_path: Path) -> None:
    source_root = tmp_path / "psyq-source"
    make_fake_psyq_tree(source_root)

    try:
        psyq_lib.find_psyq_source(source_root=source_root)
    except ValueError as exc:
        assert "must stay inside the repo workspace" in str(exc)
    else:
        raise AssertionError("expected explicit outside-repo source to be rejected")
