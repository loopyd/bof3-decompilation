"""PsyQ SDK discovery, staging helpers, and path conventions."""

from __future__ import annotations

import contextlib
import shutil
import tempfile
from dataclasses import dataclass
from pathlib import Path

from ..io import DEFAULT_PSYQ_VERSION, normalize_psyq_version
from .helpers import (
    find_matching_files,
    paths_under,
    require_path_under,
    unique_paths,
)
from .releases import (
    archive_path_looks_valid as archive_file_looks_valid,
    extract_archive,
)


REPO_ROOT = Path(__file__).resolve().parents[4]


INCLUDE_FILE_NAMES = ("LIBGPU.H", "libgpu.h")


LIB_FILE_NAMES = ("LIBGPU.LIB", "libgpu.lib", "libgpu.a")


TEXT_FILE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".s", ".txt"}


DEFAULT_PSYQ_ARCHIVE_URL = (
    "https://archive.org/download/ps1_sdks/Runtime%20Library%204.7.zip"
)


DEFAULT_PSYQ_CONVERTED_ARCHIVE_URL = (
    "https://psx.arthus.net/sdk/Psy-Q/psyq-4.7-converted-full.7z"
)


def psyq_dest(version: str | None = None) -> Path:
    return REPO_ROOT / "toolchains" / "psyq" / normalize_psyq_version(version)


def default_private_assets_root() -> Path:
    return REPO_ROOT / "inputs" / "external" / "private-assets"


def psyq_archive_stem(version: str | None = None) -> str:
    resolved_version = normalize_psyq_version(version)
    if resolved_version == DEFAULT_PSYQ_VERSION:
        return "Runtime Library 4.7"
    return f"psyq-{resolved_version}"


def default_psyq_archive_url(version: str | None = None) -> str | None:
    if normalize_psyq_version(version) == DEFAULT_PSYQ_VERSION:
        return DEFAULT_PSYQ_ARCHIVE_URL
    return None


def default_psyq_converted_archive_url(version: str | None = None) -> str | None:
    if normalize_psyq_version(version) == DEFAULT_PSYQ_VERSION:
        return DEFAULT_PSYQ_CONVERTED_ARCHIVE_URL
    return None


def psyq_private_cache_root(
    private_root: Path | None = None, version: str | None = None
) -> Path:
    return (
        (private_root or default_private_assets_root())
        / "psyq"
        / (normalize_psyq_version(version))
    )


@dataclass(frozen=True)
class PsyqSource:
    kind: str
    path: Path


def contains_any_file(directory: Path, names: tuple[str, ...]) -> bool:
    entries = {child.name.lower() for child in directory.iterdir()}
    return any(name.lower() in entries for name in names)


def find_sdk_subdir(
    source_root: Path, dir_name: str, required_files: tuple[str, ...]
) -> Path:
    for candidate in sorted(path for path in source_root.rglob("*") if path.is_dir()):
        if candidate.name.lower() != dir_name.lower():
            continue
        if contains_any_file(candidate, required_files):
            return candidate
    raise FileNotFoundError(
        f"could not find {dir_name} under {source_root} with one of: {', '.join(required_files)}"
    )


def create_lowercase_aliases(root: Path) -> None:
    for candidate in sorted(root.rglob("*")):
        alias_name = candidate.name.lower()
        if candidate.name == alias_name:
            continue
        alias_path = candidate.with_name(alias_name)
        if alias_path.exists():
            continue
        if candidate.is_dir():
            shutil.copytree(candidate, alias_path)
        else:
            shutil.copy2(candidate, alias_path)


def should_normalize_text_file(path: Path) -> bool:
    if path.suffix.lower() in TEXT_FILE_SUFFIXES:
        return True
    return path.suffix == "" and path.parent.name.lower() == "include"


def file_uses_crlf(path: Path) -> bool:
    return b"\r\n" in path.read_bytes()


def normalize_text_file_newlines(path: Path) -> bool:
    data = path.read_bytes()
    if b"\0" in data or b"\r" not in data:
        return False
    normalized = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    if normalized == data:
        return False
    path.write_bytes(normalized)
    return True


def list_text_files_with_crlf(root: Path) -> list[Path]:
    offending: list[Path] = []
    for candidate in sorted(root.rglob("*")):
        if not candidate.is_file() or candidate.is_symlink():
            continue
        if not should_normalize_text_file(candidate):
            continue
        if file_uses_crlf(candidate):
            offending.append(candidate)
    return offending


def normalize_text_tree_newlines(root: Path) -> int:
    candidates = list_text_files_with_crlf(root)
    return sum(1 for candidate in candidates if normalize_text_file_newlines(candidate))


def staged_sdk_layout_exists(root: Path) -> bool:
    return (root / "include").exists() and (root / "lib").exists()


def original_sdk_is_ready(root: Path) -> bool:
    libgpu_header = root / "include" / "libgpu.h"
    return (
        staged_sdk_layout_exists(root)
        and libgpu_header.exists()
        and not list_text_files_with_crlf(root)
    )


def source_root_looks_valid(path: Path) -> bool:
    if not path.exists() or not path.is_dir():
        return False
    try:
        _ = find_sdk_subdir(path, "INCLUDE", INCLUDE_FILE_NAMES)
        _ = find_sdk_subdir(path, "LIB", LIB_FILE_NAMES)
    except FileNotFoundError:
        return False
    return True


def auto_discovery_roots(version: str | None = None) -> list[Path]:
    stem = psyq_archive_stem(version)
    cache_root = psyq_private_cache_root(version=version)
    candidates: list[Path] = []
    candidates.extend(
        [
            REPO_ROOT / "inputs" / "external" / stem,
            REPO_ROOT / "inputs" / stem,
            cache_root / "source-tree" / stem,
            cache_root / "source-tree",
        ]
    )
    return paths_under(candidates, REPO_ROOT / "inputs")


def auto_discovery_archives(version: str | None = None) -> list[Path]:
    stem = psyq_archive_stem(version)
    cache_root = psyq_private_cache_root(version=version)
    candidates: list[Path] = []
    candidates.append(cache_root / "source-media")
    for parent in (REPO_ROOT / "inputs" / "external", REPO_ROOT / "inputs"):
        for suffix in (".7z", ".zip", ".tar.gz", ".tgz"):
            candidates.append(parent / f"{stem}{suffix}")
    return paths_under(candidates, REPO_ROOT / "inputs")


def discover_source_root(
    explicit_source: Path | None = None,
    *,
    version: str | None = None,
) -> Path | None:
    candidates: list[Path] = []
    if explicit_source is not None:
        candidates.append(
            require_path_under(
                explicit_source, REPO_ROOT / "inputs", label="PsyQ source root"
            )
        )
    candidates.extend(auto_discovery_roots(version))
    for candidate in unique_paths(candidates):
        if source_root_looks_valid(candidate):
            return candidate
    return None


def discover_source_archive(
    explicit_archive: Path | None = None,
    *,
    version: str | None = None,
) -> Path | None:
    candidates: list[Path] = []
    if explicit_archive is not None:
        candidates.append(
            require_path_under(
                explicit_archive, REPO_ROOT / "inputs", label="PsyQ archive"
            )
        )
    candidates.extend(auto_discovery_archives(version))
    for candidate in unique_paths(candidates):
        matches = find_matching_files(candidate, archive_file_looks_valid)
        if matches:
            return matches[0]
    return None


def discover_source_input(
    explicit_source: Path | None = None,
    explicit_archive: Path | None = None,
    *,
    version: str | None = None,
) -> PsyqSource | None:
    source_root = discover_source_root(explicit_source, version=version)
    if source_root is not None:
        return PsyqSource(kind="tree", path=source_root)
    archive_path = discover_source_archive(explicit_archive, version=version)
    if archive_path is not None:
        return PsyqSource(kind="archive", path=archive_path)
    return None


@contextlib.contextmanager
def materialized_source_root(source_input: PsyqSource):
    if source_input.kind == "tree":
        yield source_input.path
        return
    with tempfile.TemporaryDirectory(prefix="harness-psyq-") as tmp_dir:
        extraction_root = Path(tmp_dir) / "source"
        extraction_root.mkdir(parents=True, exist_ok=True)
        extract_archive(source_input.path, extraction_root)
        yield extraction_root


def find_psyq_source(
    *,
    source_root: Path | None = None,
    archive: Path | None = None,
    version: str | None = None,
) -> PsyqSource | None:
    psyq_version = normalize_psyq_version(version)
    return discover_source_input(source_root, archive, version=psyq_version)
