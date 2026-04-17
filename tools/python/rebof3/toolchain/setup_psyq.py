from __future__ import annotations

import contextlib
import os
import shutil
import subprocess
import tempfile
import urllib.parse
import urllib.request
from dataclasses import dataclass
from pathlib import Path

from .archive import (
    archive_path_looks_valid as archive_file_looks_valid,
    archive_stem,
    extract_archive,
    sync_archive_into_store,
)
from .setup_disc import find_disc_set


REPO_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_DEST = REPO_ROOT / "toolchains" / "psyq-original" / "4.0"

INCLUDE_FILE_NAMES = ("LIBGPU.H", "libgpu.h")
LIB_FILE_NAMES = ("LIBGPU.LIB", "libgpu.lib", "libgpu.a")
TEXT_FILE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".s", ".txt"}
DEFAULT_PRIVATE_ASSETS_ROOT = REPO_ROOT / "external" / "private-assets"
DEFAULT_PSYQ_ARCHIVE_URL = "https://archive.org/download/ps1_sdks/Programmer%20Tool%20-%20Runtime%20Library%20Version%204.0%20%28Japan%29_DTL-S2320_redump.zip"
RAW_SECTOR_SIZE = 2352
RAW_SECTOR_DATA_OFFSET = 24
RAW_SECTOR_DATA_SIZE = 2048
PRIMARY_VOLUME_DESCRIPTOR_SECTOR = 16

AUTO_DISCOVERY_CANDIDATES = (
    REPO_ROOT / "inputs" / "external" / "psyq-4.0",
    REPO_ROOT / "inputs" / "external" / "psyq40",
    REPO_ROOT / "inputs" / "psyq-4.0",
    REPO_ROOT / "inputs" / "psyq40",
)
HOME_DISCOVERY_PATTERNS = (
    "psyq-4.0",
    "psyq40",
    "PsyQ4.0",
    "Downloads/psyq-4.0",
    "Downloads/psyq40",
    "Documents/psyq-4.0",
    "Documents/psyq40",
)
AUTO_DISCOVERY_ARCHIVES = (
    REPO_ROOT / "inputs" / "external" / "psyq-4.7-converted-full.7z",
    REPO_ROOT / "inputs" / "psyq-4.7-converted-full.7z",
)
HOME_ARCHIVE_PATTERNS = (
    "Downloads/Programmer Tool - Runtime Library Version 4.0 (Japan)_DTL-S2320_redump.zip",
    "Downloads/psyq-4.7-converted-full.7z",
    "Downloads/psyq-4.7-converted-full.zip",
    "Downloads/psyq40.7z",
    "Downloads/psyq40.zip",
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


def _resolve_psyq_inputs(
    source_root: Path | None,
    archive: Path | None,
) -> tuple[Path | None, Path | None]:
    if source_root is None:
        source_env = os.environ.get("PSYQ_SOURCE") or os.environ.get("PSYQ40_SOURCE")
        if source_env:
            source_root = Path(source_env)
    if archive is None:
        archive_env = os.environ.get("PSYQ_ARCHIVE") or os.environ.get("PSYQ40_ARCHIVE")
        if archive_env:
            archive = Path(archive_env)
    return source_root, archive


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
    if not candidates:
        return 0
    if shutil.which("dos2unix") is not None:
        try:
            subprocess.run(
                ["dos2unix", "-q", *(str(candidate) for candidate in candidates)],
                check=True,
            )
            return len(candidates)
        except (OSError, subprocess.CalledProcessError):
            pass
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


def auto_discovery_roots() -> list[Path]:
    candidates: list[Path] = []
    env_source = os.environ.get("PSYQ_SOURCE") or os.environ.get("PSYQ40_SOURCE")
    if env_source:
        candidates.append(Path(env_source))
    candidates.extend(AUTO_DISCOVERY_CANDIDATES)
    home = Path.home()
    candidates.extend(home / pattern for pattern in HOME_DISCOVERY_PATTERNS)
    return _dedupe_paths(candidates)


def auto_discovery_archives() -> list[Path]:
    candidates: list[Path] = []
    env_archive = os.environ.get("PSYQ_ARCHIVE") or os.environ.get("PSYQ40_ARCHIVE")
    if env_archive:
        candidates.append(Path(env_archive))
    candidates.extend(AUTO_DISCOVERY_ARCHIVES)
    home = Path.home()
    candidates.extend(home / pattern for pattern in HOME_ARCHIVE_PATTERNS)
    return _dedupe_paths(candidates)


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


def discover_source_root(explicit_source: Path | None = None) -> Path | None:
    candidates: list[Path] = []
    if explicit_source is not None:
        candidates.append(explicit_source)
    candidates.extend(auto_discovery_roots())
    for candidate in _dedupe_paths(candidates):
        if source_root_looks_valid(candidate):
            return candidate
    return None


def discover_source_archive(explicit_archive: Path | None = None) -> Path | None:
    candidates: list[Path] = []
    if explicit_archive is not None:
        candidates.append(explicit_archive)
    candidates.extend(auto_discovery_archives())
    for candidate in _dedupe_paths(candidates):
        matches = _iter_archive_matches(candidate)
        if matches:
            return matches[0]
    return None


def discover_source_input(
    explicit_source: Path | None = None,
    explicit_archive: Path | None = None,
) -> PsyqSource | None:
    source_root = discover_source_root(explicit_source)
    if source_root is not None:
        return PsyqSource(kind="tree", path=source_root)
    archive_path = discover_source_archive(explicit_archive)
    if archive_path is not None:
        return PsyqSource(kind="archive", path=archive_path)
    return None


def _read_le_uint32(field: bytes) -> int:
    return int.from_bytes(field[:4], "little")


def materialize_raw_mode2_iso(track_path: Path, iso_path: Path) -> Path:
    track_data = track_path.read_bytes()
    if len(track_data) < RAW_SECTOR_SIZE:
        raise RuntimeError(f"raw disc track is too small: {track_path}")

    iso_path.parent.mkdir(parents=True, exist_ok=True)
    with iso_path.open("wb") as output:
        for offset in range(0, len(track_data), RAW_SECTOR_SIZE):
            sector = track_data[offset : offset + RAW_SECTOR_SIZE]
            if len(sector) != RAW_SECTOR_SIZE:
                continue
            output.write(
                sector[
                    RAW_SECTOR_DATA_OFFSET : RAW_SECTOR_DATA_OFFSET
                    + RAW_SECTOR_DATA_SIZE
                ]
            )
    return iso_path


def iter_iso_root_files(iso_path: Path) -> list[tuple[str, int, int]]:
    with iso_path.open("rb") as iso_file:
        iso_file.seek(PRIMARY_VOLUME_DESCRIPTOR_SECTOR * RAW_SECTOR_DATA_SIZE)
        pvd = iso_file.read(RAW_SECTOR_DATA_SIZE)
        if len(pvd) < 190 or pvd[0] != 1 or pvd[1:6] != b"CD001":
            raise RuntimeError(
                f"invalid ISO-9660 primary volume descriptor: {iso_path}"
            )

        root_record = pvd[156 : 156 + pvd[156]]
        root_extent = _read_le_uint32(root_record[2:10])
        root_size = _read_le_uint32(root_record[10:18])
        iso_file.seek(root_extent * RAW_SECTOR_DATA_SIZE)
        directory_data = iso_file.read(root_size)

    files: list[tuple[str, int, int]] = []
    offset = 0
    while offset < len(directory_data):
        record_length = directory_data[offset]
        if record_length == 0:
            next_sector = ((offset // RAW_SECTOR_DATA_SIZE) + 1) * RAW_SECTOR_DATA_SIZE
            offset = next_sector
            continue

        record = directory_data[offset : offset + record_length]
        name_length = record[32]
        name = record[33 : 33 + name_length].decode("ascii", errors="ignore")
        if name not in ("\x00", "\x01"):
            extent = _read_le_uint32(record[2:10])
            size = _read_le_uint32(record[10:18])
            flags = record[25]
            if flags & 0x02 == 0:
                files.append((name.split(";", 1)[0], extent, size))
        offset += record_length
    return files


def extract_iso_root_file(iso_path: Path, file_name: str, dest: Path) -> Path:
    for candidate_name, extent, size in iter_iso_root_files(iso_path):
        if candidate_name.upper() != file_name.upper():
            continue
        with iso_path.open("rb") as iso_file:
            iso_file.seek(extent * RAW_SECTOR_DATA_SIZE)
            payload = iso_file.read(size)
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(payload)
        return dest
    raise FileNotFoundError(f"could not find {file_name} in {iso_path}")


def extract_lzh_archive(archive_path: Path, dest: Path) -> None:
    dest.mkdir(parents=True, exist_ok=True)
    subprocess.run(["7z", "x", str(archive_path), f"-o{dest}"], check=True)


def prepare_original_media_source_root(source_root: Path, *, force: bool) -> Path:
    if source_root_looks_valid(source_root):
        return source_root

    cue_path, bin_paths = find_disc_set(source_root)
    if not bin_paths:
        raise FileNotFoundError(
            f"could not find a PlayStation data track under {source_root}"
        )

    prepared_root = source_root / "sdk-source"
    if force and prepared_root.exists():
        shutil.rmtree(prepared_root)
    if source_root_looks_valid(prepared_root):
        return prepared_root

    iso_path = prepared_root / "disc" / (cue_path.stem + ".iso")
    materialize_raw_mode2_iso(bin_paths[0], iso_path)

    archives_dir = prepared_root / "archives"
    for archive_name in ("PSX40.LZH", "PSYQ40.LZH"):
        try:
            extract_iso_root_file(iso_path, archive_name, archives_dir / archive_name)
            extract_lzh_archive(archives_dir / archive_name, prepared_root / "expanded")
            if source_root_looks_valid(prepared_root / "expanded"):
                return prepared_root / "expanded"
        except FileNotFoundError:
            continue

    expanded_root = prepared_root / "expanded"
    if not source_root_looks_valid(expanded_root):
        raise RuntimeError(
            f"prepared PsyQ source tree is incomplete under {expanded_root}"
        )
    return expanded_root


@contextlib.contextmanager
def materialized_source_root(source_input: PsyqSource):
    if source_input.kind == "tree":
        yield source_input.path
        return
    with tempfile.TemporaryDirectory(prefix="psyq40-") as tmp_dir:
        extraction_root = Path(tmp_dir) / "source"
        extraction_root.mkdir(parents=True, exist_ok=True)
        extract_archive(source_input.path, extraction_root)
        yield extraction_root


def find_psyq_source(
    *,
    source_root: Path | None = None,
    archive: Path | None = None,
) -> PsyqSource | None:
    source_root, archive = _resolve_psyq_inputs(source_root, archive)
    return discover_source_input(source_root, archive)


def stage_psyq_sdk(
    *,
    dest: Path = DEFAULT_DEST,
    source_root: Path | None = None,
    archive: Path | None = None,
    force: bool = False,
) -> Path:
    source_root, archive = _resolve_psyq_inputs(source_root, archive)
    source_input = discover_source_input(source_root, archive)
    if source_input is None:
        raise FileNotFoundError(
            "missing PsyQ 4.0 source tree or archive; pass --source-root/--archive, set PSYQ_SOURCE/PSYQ_ARCHIVE, or use the legacy PSYQ40_SOURCE/PSYQ40_ARCHIVE names"
        )

    dest_root = dest.resolve()
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
        raise RuntimeError(f"staged PsyQ 4.0 tree is incomplete under {dest_root}")

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
    dest: Path = DEFAULT_DEST,
    archive: Path | None = None,
    archive_url: str | None = None,
    private_assets_root: Path = DEFAULT_PRIVATE_ASSETS_ROOT,
    force: bool = False,
) -> Path:
    archive_store = private_assets_root / "psyq" / "source-media"
    if archive is not None:
        resolved_archive = discover_source_archive(archive)
        if resolved_archive is None:
            raise FileNotFoundError(f"missing PsyQ 4.0 source archive: {archive}")
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
        resolved_archive = discover_source_archive()
        if resolved_archive is None:
            raise FileNotFoundError(
                "missing PsyQ 4.0 source archive; pass --archive, provide PSYQ_ARCHIVE, or use `toolchain psyq import` to cache and stage it via the optional private-assets workspace"
            )
        source_archive = sync_archive_into_store(
            resolved_archive,
            archive_store / resolved_archive.name,
            force=force,
        )

    source_root = (
        private_assets_root / "psyq" / "source-tree" / archive_stem(source_archive)
    )
    if force and source_root.exists():
        shutil.rmtree(source_root)
    if not source_root.exists() or not any(source_root.iterdir()):
        extract_archive(source_archive, source_root)
    prepared_source_root = prepare_original_media_source_root(source_root, force=force)
    return stage_psyq_sdk(dest=dest, source_root=prepared_source_root, force=force)
