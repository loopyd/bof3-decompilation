from __future__ import annotations

import shutil
import urllib.parse
from pathlib import Path

from ..io import RepoLayout, normalize_psyq_version
from .base import Toolchain, ensure_gitkeep
from .helpers import (
    download_file,
)
from .releases import (
    archive_stem,
    extract_archive,
    sync_archive_into_store,
)

from .psyq_discovery import (
    INCLUDE_FILE_NAMES,
    LIB_FILE_NAMES,
    create_lowercase_aliases,
    default_private_assets_root,
    default_psyq_archive_url,
    default_psyq_converted_archive_url,
    discover_source_archive,
    discover_source_input,
    find_sdk_subdir,
    materialized_source_root,
    normalize_text_tree_newlines,
    original_sdk_is_ready,
    psyq_dest,
    psyq_private_cache_root,
    source_root_looks_valid,
)

# Official source media: headers and PsyQ .LIB archives.
# Converted extraction: per-object .o files required by reviewed signature evidence.

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
        source_archive = download_file(
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
        source_archive = download_file(
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
        raise FileNotFoundError(
            f"no converted PsyQ archive is configured for {psyq_version}"
        )
    archive_name = Path(urllib.parse.urlparse(archive_url).path).name
    archive = download_file(
        archive_url, cache_root / "source-media" / archive_name, force=force
    )
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
        raise RuntimeError(
            f"converted PsyQ {psyq_version} archive has no library directory"
        )
    dest_root = (dest or psyq_dest(psyq_version)).resolve()
    for source_library in library_root.iterdir():
        if source_library.is_dir():
            shutil.copytree(
                source_library,
                dest_root / source_library.name.lower(),
                dirs_exist_ok=True,
            )
    return dest_root

class PsyqToolchain(Toolchain):
    label = "PsyQ 4.7"

    def __init__(
        self,
        layout: RepoLayout,
        *,
        archive: Path | None = None,
        archive_url: str | None = None,
    ) -> None:
        self.layout = layout
        self.archive = archive
        self.archive_url = archive_url

    def install(self, *, force: bool = False) -> str:
        import_psyq_sdk(
            dest=self.layout.psyq_root,
            archive=self.archive,
            archive_url=self.archive_url,
            private_assets_root=self.layout.private_assets_dir,
            force=force,
        )
        stage_psyq_converted_sdk(
            dest=self.layout.psyq_root,
            private_assets_root=self.layout.private_assets_dir,
            force=force,
        )
        return ""

    def verify(self) -> str:
        if not original_sdk_is_ready(self.layout.psyq_root):
            raise FileNotFoundError(f"missing PsyQ SDK: {self.layout.psyq_root}")
        return "headers, libraries, reviewed objects"
