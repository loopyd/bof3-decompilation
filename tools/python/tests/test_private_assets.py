from __future__ import annotations

from pathlib import Path

from rebof3.private_assets import (
    list_git_submodule_paths,
    list_optional_submodule_paths,
    list_required_submodule_paths,
)


def test_list_required_submodule_paths_excludes_private_assets(tmp_path: Path) -> None:
    gitmodules = tmp_path / ".gitmodules"
    gitmodules.write_text(
        """
[submodule \"third_party/bof3-disk\"]
    path = third_party/bof3-disk
[submodule \"external/private-assets\"]
    path = external/private-assets
""".strip()
        + "\n",
        encoding="utf-8",
    )

    assert list_git_submodule_paths(tmp_path) == [
        "third_party/bof3-disk",
        "external/private-assets",
    ]
    assert list_required_submodule_paths(tmp_path) == ["third_party/bof3-disk"]
    assert list_optional_submodule_paths(tmp_path) == ["external/private-assets"]
