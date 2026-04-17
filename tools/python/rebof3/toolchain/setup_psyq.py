from __future__ import annotations

import contextlib
import os
import shutil
import subprocess
import tarfile
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_DEST = REPO_ROOT / "toolchains" / "psyq-original" / "4.0"

INCLUDE_FILE_NAMES = ("LIBGPU.H", "libgpu.h")
LIB_FILE_NAMES = ("LIBGPU.LIB", "libgpu.lib", "libgpu.a")
SUPPORTED_ARCHIVE_SUFFIXES = (".7z", ".zip", ".tar.gz", ".tgz")
TEXT_FILE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inc", ".inl", ".s", ".txt"}

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
    "Downloads/psyq-4.7-converted-full.7z",
    "Downloads/psyq-4.7-converted-full.zip",
    "Downloads/psyq40.7z",
    "Downloads/psyq40.zip",
)


def ensure_gitkeep(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    (root / ".gitkeep").touch(exist_ok=True)


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
    return (
        path.exists()
        and path.is_file()
        and path.name.lower().endswith(SUPPORTED_ARCHIVE_SUFFIXES)
    )


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
        if archive_path_looks_valid(candidate):
            return candidate
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


def extract_archive(archive_path: Path, dest: Path) -> None:
    archive_name = archive_path.name.lower()
    if archive_name.endswith(".zip"):
        with zipfile.ZipFile(archive_path, "r") as archive:
            archive.extractall(dest)
        return
    if archive_name.endswith((".tar.gz", ".tgz")):
        with tarfile.open(archive_path, "r:gz") as archive:
            archive.extractall(dest)
        return
    if archive_name.endswith(".7z"):
        subprocess.run(["7z", "x", str(archive_path), f"-o{dest}"], check=True)
        return
    raise ValueError(f"unsupported archive type: {archive_path}")


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
