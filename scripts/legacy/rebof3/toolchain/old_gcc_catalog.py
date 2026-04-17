from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Mapping


OLD_GCC_REPO = "decompals/old-gcc"
OLD_GCC_RELEASE_TAG = "0.13"
DEFAULT_OLD_GCC_COMPILER_SET = "tested-matrix"


@dataclass(frozen=True, slots=True)
class OldGccRelease:
    compiler_id: str
    asset_name: str

    def install_path(self, dest_root: Path) -> Path:
        return dest_root / self.compiler_id


ALL_OLD_GCC_RELEASES = (
    OldGccRelease("gcc-2.5.7-psx", "gcc-2.5.7-psx.tar.gz"),
    OldGccRelease("gcc-2.6.0-psx", "gcc-2.6.0-psx.tar.gz"),
    OldGccRelease("gcc-2.6.3-psx", "gcc-2.6.3-psx.tar.gz"),
    OldGccRelease("gcc-2.7.0-mipsel", "gcc-2.7.0.tar.gz"),
    OldGccRelease("gcc-2.7.1-mipsel", "gcc-2.7.1.tar.gz"),
    OldGccRelease("gcc-2.7.2-psx", "gcc-2.7.2-psx.tar.gz"),
    OldGccRelease("gcc-2.7.2.1-mipsel", "gcc-2.7.2.1.tar.gz"),
    OldGccRelease("gcc-2.7.2.2-mipsel", "gcc-2.7.2.2.tar.gz"),
    OldGccRelease("gcc-2.7.2.3-mipsel", "gcc-2.7.2.3.tar.gz"),
    OldGccRelease("gcc-2.8.0-psx", "gcc-2.8.0-psx.tar.gz"),
    OldGccRelease("gcc-2.8.1-psx", "gcc-2.8.1-psx.tar.gz"),
    OldGccRelease("gcc-2.91.66-psx", "gcc-2.91.66-psx.tar.gz"),
    OldGccRelease("gcc-2.95.2-psx", "gcc-2.95.2-psx.tar.gz"),
)

OLD_GCC_RELEASES_BY_ID = {
    release.compiler_id: release for release in ALL_OLD_GCC_RELEASES
}

OLD_GCC_COMPILER_SETS = {
    DEFAULT_OLD_GCC_COMPILER_SET: (
        "gcc-2.5.7-psx",
        "gcc-2.6.0-psx",
        "gcc-2.6.3-psx",
        "gcc-2.7.0-mipsel",
        "gcc-2.7.1-mipsel",
        "gcc-2.7.2.1-mipsel",
        "gcc-2.7.2.2-mipsel",
        "gcc-2.7.2.3-mipsel",
        "gcc-2.8.0-psx",
        "gcc-2.8.1-psx",
        "gcc-2.91.66-psx",
        "gcc-2.95.2-psx",
    )
}
OLD_GCC_TESTED_MATRIX_COMPILER_IDS = OLD_GCC_COMPILER_SETS[DEFAULT_OLD_GCC_COMPILER_SET]


def dedupe_compiler_ids(compiler_ids: Iterable[str]) -> tuple[str, ...]:
    return tuple(dict.fromkeys(compiler_ids))


def expand_compiler_ids(
    requested_ids: Iterable[str] | None,
    compiler_sets: Iterable[str] | None,
    *,
    default_ids: Iterable[str] = (),
    set_prefix_ids: Mapping[str, Iterable[str]] | None = None,
) -> tuple[str, ...]:
    requested = tuple(requested_ids or ())
    selected_sets = tuple(compiler_sets or ())
    if not requested and not selected_sets:
        return dedupe_compiler_ids(default_ids)

    selected: list[str] = []
    for compiler_set in selected_sets:
        if set_prefix_ids is not None:
            selected.extend(set_prefix_ids.get(compiler_set, ()))
        selected.extend(OLD_GCC_COMPILER_SETS[compiler_set])
    selected.extend(requested)
    return dedupe_compiler_ids(selected)


def release_for_compiler(compiler_id: str) -> OldGccRelease:
    try:
        return OLD_GCC_RELEASES_BY_ID[compiler_id]
    except KeyError as exc:
        raise KeyError(f"unknown old-gcc compiler id: {compiler_id}") from exc
