from pathlib import Path

from harness.paths import repo_layout


def test_repo_layout_uses_canonical_rust_extraction_tools(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)

    assert layout.harness_disk_src == tmp_path / "third_party" / "bof3-disk-v2"
    assert layout.emi_ex_src == tmp_path / "third_party" / "emi-ex-v2"
    assert layout.harness_disk_bin == (
        tmp_path / "build/tools/rust/bof3-disk/release/bof3-disk"
    )
    assert layout.emi_ex_bin == tmp_path / "build/tools/rust/emi-ex/release/emi-ex"
