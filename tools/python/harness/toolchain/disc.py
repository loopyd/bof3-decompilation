from __future__ import annotations

import re
import shutil
import urllib.parse
import urllib.request
from dataclasses import dataclass
from pathlib import Path

from .archive import (
    archive_path_looks_valid,
    archive_stem,
    extract_archive,
    sync_archive_into_store,
)


REPO_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_DISC_DIR = REPO_ROOT / "inputs" / "external"
DEFAULT_PRIVATE_ASSETS_ROOT = DEFAULT_DISC_DIR / "private-assets"
DEFAULT_BOF3_ARCHIVE_URL = "https://archive.org/download/BreathOfFireIIIv1.1.7z"

AUTO_DISCOVERY_ARCHIVES = (
    DEFAULT_DISC_DIR / "BreathOfFireIIIv1.1.7z",
)

FILE_PATTERN = re.compile(r'^\s*FILE\s+"([^"]+)"\s+\S+', re.IGNORECASE)


@dataclass(frozen=True)
class DiscImportResult:
    archive_path: Path
    extracted_root: Path
    cue_path: Path
    staged_paths: tuple[Path, ...]


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


def _validate_repo_local_input(path: Path, *, label: str) -> Path:
    if not _is_allowed_input_path(path):
        raise ValueError(f"{label} must stay under the repo's inputs/ tree: {path}")
    return path.expanduser()


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


def discover_disc_archive(explicit_archive: Path | None = None) -> Path | None:
    candidates: list[Path] = []
    if explicit_archive is not None:
        candidates.append(
            _validate_repo_local_input(explicit_archive, label="BOF3 archive")
        )
    candidates.extend(AUTO_DISCOVERY_ARCHIVES)

    for candidate in _dedupe_paths(
        [candidate for candidate in candidates if _is_allowed_input_path(candidate)]
    ):
        matches = _iter_archive_matches(candidate)
        if matches:
            return matches[0]
    return None


def download_archive(url: str, dest: Path, *, force: bool) -> Path:
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.exists() and not force:
        return dest
    with urllib.request.urlopen(url) as response, dest.open("wb") as output:
        shutil.copyfileobj(response, output)
    return dest


def resolve_bof3_archive(
    *,
    archive: Path | None = None,
    archive_url: str | None = None,
    private_assets_root: Path = DEFAULT_PRIVATE_ASSETS_ROOT,
    force: bool = False,
) -> Path:
    archive_store = private_assets_root / "bof3" / "source-media"
    if archive is not None:
        resolved_archive = discover_disc_archive(archive)
        if resolved_archive is None:
            raise FileNotFoundError(f"missing BOF3 archive: {archive}")
        return sync_archive_into_store(
            resolved_archive,
            archive_store / resolved_archive.name,
            force=force,
        )

    if archive_url is not None:
        archive_name = Path(urllib.parse.urlparse(archive_url).path).name
        if not archive_name:
            raise ValueError(f"could not derive archive name from URL: {archive_url}")
        return download_archive(archive_url, archive_store / archive_name, force=force)

    resolved_archive = discover_disc_archive()
    if resolved_archive is None:
        raise FileNotFoundError(
            "missing BOF3 archive; pass --archive or use `toolchain disc import` to download, cache, and stage it via the optional private-assets workspace"
        )
    return sync_archive_into_store(
        resolved_archive,
        archive_store / resolved_archive.name,
        force=force,
    )


def parse_cue_bin_paths(cue_path: Path) -> list[Path]:
    matches: list[Path] = []
    for line in cue_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        match = FILE_PATTERN.match(line)
        if match is None:
            continue
        matches.append((cue_path.parent / match.group(1)).resolve())
    return matches


def find_disc_set(root: Path) -> tuple[Path, list[Path]]:
    candidates: list[tuple[tuple[int, str], Path, list[Path]]] = []
    for cue_path in sorted(root.rglob("*.cue")):
        bin_paths = parse_cue_bin_paths(cue_path)
        if not bin_paths or not all(path.exists() for path in bin_paths):
            continue
        relative_parent = cue_path.parent.relative_to(root)
        sort_key = (len(relative_parent.parts), str(cue_path.relative_to(root)).lower())
        candidates.append((sort_key, cue_path, bin_paths))

    if not candidates:
        raise FileNotFoundError(
            f"could not find a complete BOF3 cue/bin set under {root}"
        )

    _, cue_path, bin_paths = min(candidates, key=lambda item: item[0])
    return cue_path, bin_paths


def import_bof3_disc(
    *,
    dest: Path = DEFAULT_DISC_DIR,
    archive: Path | None = None,
    archive_url: str | None = None,
    private_assets_root: Path = DEFAULT_PRIVATE_ASSETS_ROOT,
    force: bool = False,
) -> DiscImportResult:
    archive_path = resolve_bof3_archive(
        archive=archive,
        archive_url=archive_url,
        private_assets_root=private_assets_root,
        force=force,
    )
    extracted_root = (
        private_assets_root / "bof3" / "source-tree" / archive_stem(archive_path)
    )
    if force and extracted_root.exists():
        shutil.rmtree(extracted_root)
    if not extracted_root.exists() or not any(extracted_root.iterdir()):
        extract_archive(archive_path, extracted_root)

    cue_path, bin_paths = find_disc_set(extracted_root)
    return DiscImportResult(
        archive_path=archive_path,
        extracted_root=extracted_root,
        cue_path=cue_path,
        staged_paths=(cue_path, *bin_paths),
    )
