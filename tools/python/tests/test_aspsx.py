from __future__ import annotations

import tarfile
from pathlib import Path

from rebof3.paths import repo_layout
from rebof3.toolchain import aspsx as module


def create_archive(archive_path: Path, version: str) -> None:
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    temp_root = archive_path.parent / version
    temp_root.mkdir(parents=True, exist_ok=True)
    exe_path = temp_root / "ASPSX.EXE"
    exe_path.write_bytes(b"fake")
    with tarfile.open(archive_path, "w:gz") as archive:
        archive.add(temp_root, arcname=version)


def test_download_aspsx_binaries_uses_public_asset_layout(
    tmp_path: Path, monkeypatch
) -> None:
    layout = repo_layout(tmp_path)
    layout.aspsx_psyq_root.mkdir(parents=True, exist_ok=True)

    def fake_download_file(url: str, dest: Path) -> Path:
        version = dest.name.removesuffix(".tar.gz")
        create_archive(dest, version)
        return dest

    monkeypatch.setattr(module, "download_file", fake_download_file)

    result = module.download_aspsx_binaries(layout)

    assert result.root == layout.aspsx_psyq_root
    assert result.versions == module.DEFAULT_ASPSX_PSYQ_VERSIONS
    for version in module.DEFAULT_ASPSX_PSYQ_VERSIONS:
        assert (layout.aspsx_psyq_root / version / "ASPSX.EXE").exists()
        assert (layout.aspsx_psyq_compat_root / version / "ASPSX.EXE").exists()
    for version in set(module.ALL_ASPSX_PSYQ_VERSIONS) - set(result.versions):
        assert not (layout.aspsx_psyq_root / version).exists()

    assert layout.aspsx_psyq_compat_root.is_dir()


def test_download_aspsx_binaries_can_fetch_all_versions(
    tmp_path: Path, monkeypatch
) -> None:
    layout = repo_layout(tmp_path)
    layout.aspsx_psyq_root.mkdir(parents=True, exist_ok=True)

    def fake_download_file(url: str, dest: Path) -> Path:
        version = dest.name.removesuffix(".tar.gz")
        create_archive(dest, version)
        return dest

    monkeypatch.setattr(module, "download_file", fake_download_file)

    result = module.download_aspsx_binaries(
        layout,
        versions=module.ALL_ASPSX_PSYQ_VERSIONS,
    )

    assert result.versions == module.ALL_ASPSX_PSYQ_VERSIONS
    for version in module.ALL_ASPSX_PSYQ_VERSIONS:
        assert (layout.aspsx_psyq_root / version / "ASPSX.EXE").exists()
