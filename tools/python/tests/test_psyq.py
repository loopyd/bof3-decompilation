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


def test_import_psyq_sdk_stages_original_archive(tmp_path: Path) -> None:
    archive_path = tmp_path / "psyq40.zip"
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
    assert (private_root / "psyq" / "source-media" / archive_path.name).exists()
    assert (dest_root / "include" / "libgpu.h").exists()


def test_find_psyq_source_does_not_treat_private_assets_as_runtime_source(
    monkeypatch, tmp_path: Path
) -> None:
    private_root = tmp_path / "inputs" / "external" / "private-assets" / "psyq"
    make_fake_psyq_tree(private_root / "sdk-source")

    monkeypatch.setattr(
        psyq_lib,
        "AUTO_DISCOVERY_CANDIDATES",
        (
            tmp_path / "inputs" / "external" / "psyq-4.0",
            tmp_path / "inputs" / "external" / "psyq40",
            tmp_path / "inputs" / "psyq-4.0",
            tmp_path / "inputs" / "psyq40",
        ),
    )
    monkeypatch.setattr(
        psyq_lib,
        "AUTO_DISCOVERY_ARCHIVES",
        (
            tmp_path / "inputs" / "external" / "psyq-4.7-converted-full.7z",
            tmp_path / "inputs" / "psyq-4.7-converted-full.7z",
        ),
    )
    monkeypatch.setattr(psyq_lib, "HOME_DISCOVERY_PATTERNS", ())
    monkeypatch.setattr(psyq_lib, "HOME_ARCHIVE_PATTERNS", ())

    assert psyq_lib.find_psyq_source() is None
