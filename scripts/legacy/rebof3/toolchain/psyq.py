from __future__ import annotations

import argparse
import contextlib
import os
import shutil
import subprocess
import tarfile
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path

from ..cli import add_logging_args, context_from_args, package_prog
from ..config import PSYQ_ORIGINAL_40_ROOT, ROOT
from .installer import Installer


INCLUDE_FILE_NAMES = ("LIBGPU.H", "libgpu.h")
LIB_FILE_NAMES = ("LIBGPU.LIB", "libgpu.lib", "libgpu.a")
AUTO_DISCOVERY_CANDIDATES = (
    ROOT / "psyq-4.0",
    ROOT / "psyq40",
    ROOT / "deps" / "psyq-4.0",
    ROOT / "deps" / "psyq40",
    ROOT / "deps" / "psyq-original-src",
    ROOT / "deps" / "psyq-original" / "4.0-src",
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
    ROOT / "psyq-4.7-converted-full.7z",
    ROOT / "psyq-4.0-converted-full.7z",
    ROOT / "deps" / "psyq-4.7-converted-full.7z",
    ROOT / "deps" / "psyq-4.0-converted-full.7z",
)
HOME_ARCHIVE_PATTERNS = (
    "Downloads/psyq-4.7-converted-full.7z",
    "Downloads/psyq-4.7-converted-full.zip",
    "Downloads/psyq-4.0-converted-full.7z",
    "Downloads/psyq-4.0-converted-full.zip",
    "Downloads/psyq40.7z",
    "Downloads/psyq40.zip",
)
SUPPORTED_ARCHIVE_SUFFIXES = (".7z", ".zip", ".tar.gz", ".tgz")
TEXT_FILE_SUFFIXES = {
    ".c",
    ".cc",
    ".cpp",
    ".h",
    ".hpp",
    ".inc",
    ".inl",
    ".s",
    ".txt",
}
STAGED_SDK_REQUIRED_DIRS = ("include", "lib")


@dataclass(frozen=True, slots=True)
class PsyqOriginalStageRequest:
    source_root: Path
    dest: Path
    force: bool = False


@dataclass(frozen=True, slots=True)
class PsyqSourceInput:
    kind: str
    path: Path


class PsyqOriginalStager(Installer):
    installer_name = "psyq_original"

    def stage(self, request: PsyqOriginalStageRequest, *, logger) -> int:
        source_root = request.source_root.resolve()
        dest_root = request.dest.resolve()
        if not source_root.exists():
            logger.error(f"source root not found: {source_root}")
            return 1
        if original_sdk_is_ready(dest_root) and not request.force:
            logger.summary(f"PsyQ 4.0 already staged at {dest_root}")
            return 0
        try:
            include_source = find_sdk_subdir(source_root, "INCLUDE", INCLUDE_FILE_NAMES)
            lib_source = find_sdk_subdir(source_root, "LIB", LIB_FILE_NAMES)
        except FileNotFoundError as exc:
            logger.error(str(exc))
            return 1
        shutil.rmtree(dest_root, ignore_errors=True)
        (dest_root / "include").parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(include_source, dest_root / "include")
        shutil.copytree(lib_source, dest_root / "lib")
        normalize_text_tree_newlines(dest_root)
        create_lowercase_aliases(dest_root / "include")
        create_lowercase_aliases(dest_root / "lib")
        if not original_sdk_is_ready(dest_root):
            logger.error(
                "staged PsyQ 4.0 tree is incomplete; expected include/lib layout was not produced"
            )
            return 1
        logger.summary(f"PsyQ 4.0 staged at {dest_root}")
        return 0

    def install(self, request: PsyqOriginalStageRequest, *, logger) -> int:
        return self.stage(request, logger=logger)


DEFAULT_PSYQ_ORIGINAL_STAGER = PsyqOriginalStager()


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
        alias_path.symlink_to(candidate.name, target_is_directory=candidate.is_dir())


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
    normalized_count = 0
    for candidate in candidates:
        if normalize_text_file_newlines(candidate):
            normalized_count += 1
    return normalized_count


def staged_sdk_layout_exists(root: Path) -> bool:
    return all((root / subdir_name).exists() for subdir_name in STAGED_SDK_REQUIRED_DIRS)


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


def repair_staged_sdk_root(root: Path) -> int:
    normalized_count = normalize_text_tree_newlines(root)
    include_root = root / "include"
    lib_root = root / "lib"
    if include_root.exists():
        create_lowercase_aliases(include_root)
    if lib_root.exists():
        create_lowercase_aliases(lib_root)
    return normalized_count


def discover_staged_sdk_roots(scan_root: Path) -> list[Path]:
    roots: list[Path] = []
    if staged_sdk_layout_exists(scan_root):
        roots.append(scan_root)
    for candidate in sorted(path for path in scan_root.rglob("*") if path.is_dir()):
        if staged_sdk_layout_exists(candidate):
            roots.append(candidate)
    deduped: list[Path] = []
    seen: set[Path] = set()
    for candidate in roots:
        resolved = candidate.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        deduped.append(candidate)
    return deduped


def repair_staged_sdk_roots(scan_root: Path, *, logger) -> int:
    if not scan_root.exists():
        logger.error(f"repair root not found: {scan_root}")
        return 1
    sdk_roots = discover_staged_sdk_roots(scan_root)
    if not sdk_roots:
        logger.error(f"no staged PsyQ SDK roots found under {scan_root}")
        return 1
    total_normalized = 0
    repaired_roots = 0
    for sdk_root in sdk_roots:
        normalized_count = repair_staged_sdk_root(sdk_root)
        total_normalized += normalized_count
        repaired_roots += 1
        logger.item(f"repaired {sdk_root} ({normalized_count} files normalized)")
    logger.summary(
        f"repaired {repaired_roots} staged PsyQ roots under {scan_root} ({total_normalized} files normalized)"
    )
    return 0


def original_sdk_is_ready(root: Path) -> bool:
    include_root = root / "include"
    libgpu_header = include_root / "libgpu.h"
    if not staged_sdk_layout_exists(root) or not libgpu_header.exists():
        return False
    return not list_text_files_with_crlf(root)


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
    if not path.exists() or not path.is_file():
        return False
    path_text = path.name.lower()
    return path_text.endswith(SUPPORTED_ARCHIVE_SUFFIXES)


def auto_discovery_roots() -> list[Path]:
    roots: list[Path] = []
    env_source = os.environ.get("PSYQ40_SOURCE")
    if env_source:
        roots.append(Path(env_source).expanduser())
    roots.extend(AUTO_DISCOVERY_CANDIDATES)
    home = Path.home()
    roots.extend(home / pattern for pattern in HOME_DISCOVERY_PATTERNS)
    deduped: list[Path] = []
    seen: set[Path] = set()
    for candidate in roots:
        resolved = candidate.expanduser()
        if resolved in seen:
            continue
        seen.add(resolved)
        deduped.append(resolved)
    return deduped


def auto_discovery_archives() -> list[Path]:
    archives: list[Path] = []
    env_archive = os.environ.get("PSYQ40_ARCHIVE")
    if env_archive:
        archives.append(Path(env_archive).expanduser())
    archives.extend(AUTO_DISCOVERY_ARCHIVES)
    home = Path.home()
    archives.extend(home / pattern for pattern in HOME_ARCHIVE_PATTERNS)
    deduped: list[Path] = []
    seen: set[Path] = set()
    for candidate in archives:
        resolved = candidate.expanduser()
        if resolved in seen:
            continue
        seen.add(resolved)
        deduped.append(resolved)
    return deduped


def discover_source_root(explicit_source: Path | None = None) -> Path | None:
    candidates: list[Path] = []
    if explicit_source is not None:
        candidates.append(explicit_source.expanduser())
    candidates.extend(auto_discovery_roots())
    seen: set[Path] = set()
    for candidate in candidates:
        if candidate in seen:
            continue
        seen.add(candidate)
        if source_root_looks_valid(candidate):
            return candidate
    return None


def discover_source_archive(explicit_archive: Path | None = None) -> Path | None:
    candidates: list[Path] = []
    if explicit_archive is not None:
        candidates.append(explicit_archive.expanduser())
    candidates.extend(auto_discovery_archives())
    seen: set[Path] = set()
    for candidate in candidates:
        if candidate in seen:
            continue
        seen.add(candidate)
        if archive_path_looks_valid(candidate):
            return candidate
    return None


def discover_source_input(
    explicit_source: Path | None = None,
    explicit_archive: Path | None = None,
) -> PsyqSourceInput | None:
    source_root = discover_source_root(explicit_source)
    if source_root is not None:
        return PsyqSourceInput(kind="tree", path=source_root)
    archive_path = discover_source_archive(explicit_archive)
    if archive_path is not None:
        return PsyqSourceInput(kind="archive", path=archive_path)
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
def materialized_source_root(source_input: PsyqSourceInput):
    if source_input.kind == "tree":
        yield source_input.path
        return
    with tempfile.TemporaryDirectory(prefix="psyq40-") as tmp_dir:
        extraction_root = Path(tmp_dir) / "source"
        extraction_root.mkdir(parents=True, exist_ok=True)
        extract_archive(source_input.path, extraction_root)
        yield extraction_root


def build_original_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("toolchain", "psyq-original"),
        description=(
            "Stage a local original PsyQ 4.0 SDK tree into deps/psyq-original/4.0 "
            "with lowercase header aliases for Linux builds."
        ),
    )
    add_logging_args(parser)
    parser.add_argument(
        "--source-root",
        type=Path,
        help="path to an extracted/local PsyQ 4.0 tree containing INCLUDE and LIB directories",
    )
    parser.add_argument(
        "--archive",
        type=Path,
        help="path to a local converted PsyQ archive (.7z, .zip, .tar.gz, .tgz)",
    )
    parser.add_argument(
        "--no-auto-detect",
        action="store_true",
        help="require --source-root or PSYQ40_SOURCE instead of searching common local paths",
    )
    parser.add_argument(
        "--print-detected-source",
        action="store_true",
        help="print the discovered PsyQ 4.0 source root and exit",
    )
    parser.add_argument(
        "--dest",
        type=Path,
        default=PSYQ_ORIGINAL_40_ROOT,
        help="destination root for the staged original PsyQ 4.0 tree",
    )
    parser.add_argument(
        "--repair-existing",
        action="store_true",
        help="normalize CRLF text files in the staged SDK at --dest and exit",
    )
    parser.add_argument(
        "--repair-all-under",
        type=Path,
        default=None,
        help="normalize CRLF text files in every staged PsyQ SDK found under this directory and exit",
    )
    parser.add_argument("--force", action="store_true")
    return parser


def parse_original_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_original_parser().parse_args(argv)


def main_original(argv: list[str] | None = None) -> int:
    args = parse_original_args(argv)
    context = context_from_args(args, "toolchain_psyq_original")
    if args.repair_all_under is not None:
        return repair_staged_sdk_roots(args.repair_all_under, logger=context.logger)
    if args.repair_existing:
        return repair_staged_sdk_roots(args.dest, logger=context.logger)
    source_root = args.source_root
    source_input: PsyqSourceInput | None
    if args.no_auto_detect:
        if source_root is not None:
            source_input = PsyqSourceInput(kind="tree", path=source_root)
        elif args.archive is not None:
            source_input = PsyqSourceInput(kind="archive", path=args.archive)
        else:
            context.logger.error(
                "missing PsyQ 4.0 source tree; pass --source-root, --archive, or set PSYQ40_SOURCE/PSYQ40_ARCHIVE"
            )
            return 1
    else:
        source_input = discover_source_input(source_root, args.archive)
        if source_input is None:
            context.logger.error(
                "missing PsyQ 4.0 source tree or archive; pass --source-root/--archive, set PSYQ40_SOURCE/PSYQ40_ARCHIVE, or place a local tree/archive in a common path like ~/Downloads/psyq-4.7-converted-full.7z"
            )
            return 1
    if args.print_detected_source:
        print(source_input.path)
        return 0
    with materialized_source_root(source_input) as resolved_source_root:
        return DEFAULT_PSYQ_ORIGINAL_STAGER.stage(
            PsyqOriginalStageRequest(
                source_root=resolved_source_root,
                dest=args.dest,
                force=args.force,
            ),
            logger=context.logger,
        )


if __name__ == "__main__":
    raise SystemExit(main_original())
