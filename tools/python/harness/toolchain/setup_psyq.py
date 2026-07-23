from __future__ import annotations

import contextlib
import shutil
import tempfile
import urllib.parse
import urllib.request
from dataclasses import dataclass
from pathlib import Path

from ..io import DEFAULT_PSYQ_VERSION, normalize_psyq_version
from .archive import (
    archive_path_looks_valid as archive_file_looks_valid,
    archive_stem,
    extract_archive,
    sync_archive_into_store,
)


REPO_ROOT = Path(__file__).resolve().parents[4]

INCLUDE_FILE_NAMES = ("LIBGPU.H", "libgpu.h")
LIB_FILE_NAMES = ("LIBGPU.LIB", "libgpu.lib", "libgpu.a")
TEXT_FILE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".s", ".txt"}
# Official source media: headers and PsyQ .LIB archives.
DEFAULT_PSYQ_ARCHIVE_URL = "https://archive.org/download/ps1_sdks/Runtime%20Library%204.7.zip"
# Converted extraction: per-object .o files required by reviewed signature evidence.
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


def ensure_gitkeep(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    gitkeep_path = root / ".gitkeep"
    if not gitkeep_path.exists() or gitkeep_path.read_text(encoding="utf-8") != "\n":
        gitkeep_path.write_text("\n", encoding="utf-8")


@dataclass(frozen=True)
class PsyqSource:
    kind: str
    path: Path


def _dedupe_paths(candidates: list[Path]) -> list[Path]:
    deduped: list[Path] = []
    seen: set[Path] = set()
    for candidate in candidates:
        expanded = candidate.expanduser()
        if expanded in seen:
            continue
        seen.add(expanded)
        deduped.append(expanded)
    return deduped


def _is_allowed_input_path(path: Path) -> bool:
    candidate = path.expanduser().resolve(strict=False)
    inputs_root = (REPO_ROOT / "inputs").resolve()
    return candidate == inputs_root or inputs_root in candidate.parents


def _filter_repo_local_paths(candidates: list[Path]) -> list[Path]:
    return _dedupe_paths(
        [candidate for candidate in candidates if _is_allowed_input_path(candidate)]
    )


def _validate_repo_local_input(path: Path, *, label: str) -> Path:
    if not _is_allowed_input_path(path):
        raise ValueError(f"{label} must stay under the repo's inputs/ tree: {path}")
    return path.expanduser()


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


def archive_path_looks_valid(path: Path) -> bool:
    return archive_file_looks_valid(path)


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
    return _filter_repo_local_paths(candidates)


def auto_discovery_archives(version: str | None = None) -> list[Path]:
    stem = psyq_archive_stem(version)
    cache_root = psyq_private_cache_root(version=version)
    candidates: list[Path] = []
    candidates.append(cache_root / "source-media")
    for parent in (REPO_ROOT / "inputs" / "external", REPO_ROOT / "inputs"):
        for suffix in (".7z", ".zip", ".tar.gz", ".tgz"):
            candidates.append(parent / f"{stem}{suffix}")
    return _filter_repo_local_paths(candidates)


def _iter_archive_matches(candidate: Path) -> list[Path]:
    if archive_path_looks_valid(candidate):
        return [candidate]
    if not candidate.exists() or not candidate.is_dir():
        return []
    matches: list[Path] = []
    for path in sorted(candidate.rglob("*")):
        if archive_path_looks_valid(path):
            matches.append(path)
    return matches


def discover_source_root(
    explicit_source: Path | None = None,
    *,
    version: str | None = None,
) -> Path | None:
    candidates: list[Path] = []
    if explicit_source is not None:
        candidates.append(
            _validate_repo_local_input(explicit_source, label="PsyQ source root")
        )
    candidates.extend(auto_discovery_roots(version))
    for candidate in _dedupe_paths(candidates):
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
            _validate_repo_local_input(explicit_archive, label="PsyQ archive")
        )
    candidates.extend(auto_discovery_archives(version))
    for candidate in _dedupe_paths(candidates):
        matches = _iter_archive_matches(candidate)
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


def stage_psyq_sdk(
    *,
    dest: Path | None = None,
    source_root: Path | None = None,
    archive: Path | None = None,
    version: str | None = None,
    force: bool = False,
) -> Path:
    psyq_version = normalize_psyq_version(version)
    source_input = discover_source_input(source_root, archive, version=psyq_version)
    if source_input is None:
        raise FileNotFoundError(
            f"missing PsyQ {psyq_version} source tree or archive under inputs/; pass --source-root or --archive with a path under inputs/"
        )

    dest_root = (dest or psyq_dest(psyq_version)).resolve()
    if original_sdk_is_ready(dest_root) and not force:
        return dest_root

    with materialized_source_root(source_input) as resolved_source_root:
        include_source = find_sdk_subdir(
            resolved_source_root, "INCLUDE", INCLUDE_FILE_NAMES
        )
        lib_source = find_sdk_subdir(resolved_source_root, "LIB", LIB_FILE_NAMES)

        shutil.rmtree(dest_root, ignore_errors=True)
        dest_root.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(include_source, dest_root / "include")
        shutil.copytree(lib_source, dest_root / "lib")
        normalize_text_tree_newlines(dest_root)
        create_lowercase_aliases(dest_root / "include")
        create_lowercase_aliases(dest_root / "lib")
        ensure_gitkeep(dest_root)

    if not original_sdk_is_ready(dest_root):
        raise RuntimeError(
            f"staged PsyQ {psyq_version} tree is incomplete under {dest_root}"
        )

    return dest_root


def download_archive(url: str, dest: Path, *, force: bool) -> Path:
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.exists() and not force:
        return dest
    with urllib.request.urlopen(url) as response, dest.open("wb") as output:
        shutil.copyfileobj(response, output)
    return dest


def import_psyq_sdk(
    *,
    dest: Path | None = None,
    archive: Path | None = None,
    archive_url: str | None = None,
    private_assets_root: Path | None = None,
    version: str | None = None,
    force: bool = False,
) -> Path:
    psyq_version = normalize_psyq_version(version)
    resolved_private_assets_root = private_assets_root or default_private_assets_root()
    cache_root = psyq_private_cache_root(resolved_private_assets_root, psyq_version)
    archive_store = cache_root / "source-media"
    if archive is not None:
        resolved_archive = discover_source_archive(archive, version=psyq_version)
        if resolved_archive is None:
            raise FileNotFoundError(
                f"missing PsyQ {psyq_version} source archive: {archive}"
            )
        source_archive = sync_archive_into_store(
            resolved_archive,
            archive_store / resolved_archive.name,
            force=force,
        )
    elif archive_url is not None:
        archive_name = Path(urllib.parse.urlparse(archive_url).path).name
        if not archive_name:
            raise ValueError(f"could not derive archive name from URL: {archive_url}")
        source_archive = download_archive(
            archive_url,
            archive_store / archive_name,
            force=force,
        )
    else:
        archive_url = default_psyq_archive_url(psyq_version)
        if archive_url is None:
            raise FileNotFoundError(
                f"missing PsyQ {psyq_version} source archive; pass --archive or --archive-url"
            )
        archive_name = Path(urllib.parse.urlparse(archive_url).path).name
        if not archive_name:
            raise ValueError(f"could not derive archive name from URL: {archive_url}")
        source_archive = download_archive(
            archive_url,
            archive_store / archive_name,
            force=force,
        )

    source_root = cache_root / "source-tree" / archive_stem(source_archive)
    if force and source_root.exists():
        shutil.rmtree(source_root)
    if not source_root.exists() or not any(source_root.iterdir()):
        extract_archive(source_archive, source_root)
    if not source_root_looks_valid(source_root):
        raise RuntimeError(
            f"extracted PsyQ {psyq_version} source tree is incomplete under {source_root}"
        )
    return stage_psyq_sdk(
        dest=dest,
        source_root=source_root,
        version=psyq_version,
        force=force,
    )


def stage_psyq_converted_sdk(
    *,
    dest: Path | None = None,
    private_assets_root: Path | None = None,
    version: str | None = None,
    force: bool = False,
) -> Path:
    """Stage converted per-object members needed by reviewed SDK evidence."""
    psyq_version = normalize_psyq_version(version)
    private_root = private_assets_root or default_private_assets_root()
    cache_root = psyq_private_cache_root(private_root, psyq_version)
    archive_url = default_psyq_converted_archive_url(psyq_version)
    if archive_url is None:
        raise FileNotFoundError(f"no converted PsyQ archive is configured for {psyq_version}")
    archive_name = Path(urllib.parse.urlparse(archive_url).path).name
    archive = download_archive(archive_url, cache_root / "source-media" / archive_name, force=force)
    source_root = cache_root / "source-tree" / archive_stem(archive)
    if force and source_root.exists():
        shutil.rmtree(source_root)
    if not source_root.exists() or not any(source_root.iterdir()):
        extract_archive(archive, source_root)
    library_root = next(
        (
            candidate
            for candidate in sorted(source_root.rglob("*"))
            if candidate.is_dir()
            and candidate.name.lower() == "lib"
            and any(child.is_dir() for child in candidate.iterdir())
        ),
        None,
    )
    if library_root is None:
        raise RuntimeError(f"converted PsyQ {psyq_version} archive has no library directory")
    dest_root = (dest or psyq_dest(psyq_version)).resolve()
    for source_library in library_root.iterdir():
        if source_library.is_dir():
            shutil.copytree(source_library, dest_root / source_library.name.lower(), dirs_exist_ok=True)
    return dest_root
