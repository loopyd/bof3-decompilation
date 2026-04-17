from __future__ import annotations

from pathlib import Path

from rebof3.paths import repo_layout
from rebof3.toolchain import psx as module


def test_install_canonical_psx_toolchain_preserves_gitkeep_files(
    tmp_path: Path, monkeypatch
) -> None:
    layout = repo_layout(tmp_path)

    def fake_download_file(url: str, dest: Path) -> Path:
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(b"archive")
        return dest

    def fake_extract_zip(_archive: Path, dest: Path) -> None:
        if dest == layout.psn00b_toolchain_root:
            (dest / "bin").mkdir(parents=True, exist_ok=True)
            (dest / "bin" / "mipsel-none-elf-gcc").write_text("", encoding="utf-8")
            return

        sdk_root = dest / "PSn00bSDK-0.24-Linux"
        (sdk_root / "bin").mkdir(parents=True, exist_ok=True)
        (sdk_root / "bin" / "mipsel-none-elf-gcc").write_text("", encoding="utf-8")

    def fake_extract_tar_gz(_archive: Path, dest: Path) -> None:
        dest.mkdir(parents=True, exist_ok=True)
        (dest / "gcc").write_text("", encoding="utf-8")

    monkeypatch.setattr(module, "download_file", fake_download_file)
    monkeypatch.setattr(module, "extract_zip", fake_extract_zip)
    monkeypatch.setattr(module, "extract_tar_gz", fake_extract_tar_gz)

    result = module.install_canonical_psx_toolchain(layout)

    assert result.psn00b_toolchain == layout.psn00b_toolchain_root
    assert result.psn00b_sdk == layout.psn00b_sdk_root
    assert result.gcc272_psx == layout.gcc272_psx_root
    assert (layout.psn00b_toolchain_root / ".gitkeep").exists()
    assert (layout.psn00b_sdk_root / ".gitkeep").exists()
    assert (layout.gcc272_psx_root / ".gitkeep").exists()
