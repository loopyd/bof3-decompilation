from ..io import DEFAULT_PSYQ_VERSION, RepoLayout, normalize_psyq_version
from .base import Toolchain, ensure_gitkeep
from .helpers import (
    download_file,
    find_matching_files,
    paths_under,
    require_path_under,
    unique_paths,
)
from .releases import (
# Official source media: headers and PsyQ .LIB archives.
DEFAULT_PSYQ_ARCHIVE_URL = (
    "https://archive.org/download/ps1_sdks/Runtime%20Library%204.7.zip"
)
# Converted extraction: per-object .o files required by reviewed signature evidence.
DEFAULT_PSYQ_CONVERTED_ARCHIVE_URL = (
    "https://psx.arthus.net/sdk/Psy-Q/psyq-4.7-converted-full.7z"
)
        return "Runtime Library 4.7"
def default_psyq_converted_archive_url(version: str | None = None) -> str | None:
        return DEFAULT_PSYQ_CONVERTED_ARCHIVE_URL
    return None


    return paths_under(candidates, REPO_ROOT / "inputs")
    return paths_under(candidates, REPO_ROOT / "inputs")
            require_path_under(
                explicit_source, REPO_ROOT / "inputs", label="PsyQ source root"
            )
    for candidate in unique_paths(candidates):
            require_path_under(
                explicit_archive, REPO_ROOT / "inputs", label="PsyQ archive"
            )
    for candidate in unique_paths(candidates):
        matches = find_matching_files(candidate, archive_path_looks_valid)
        source_archive = download_file(
        archive_url = default_psyq_archive_url(psyq_version)
                f"missing PsyQ {psyq_version} source archive; pass --archive or --archive-url"
        source_archive = download_file(


def stage_psyq_converted_sdk(
    *,
    dest: Path | None = None,
    """Stage converted per-object members needed by reviewed SDK evidence."""
    private_root = private_assets_root or default_private_assets_root()
    cache_root = psyq_private_cache_root(private_root, psyq_version)
    archive_url = default_psyq_converted_archive_url(psyq_version)
    if archive_url is None:
            f"no converted PsyQ archive is configured for {psyq_version}"
        )
    archive = download_file(
        archive_url, cache_root / "source-media" / archive_name, force=force
    )
    source_root = cache_root / "source-tree" / archive_stem(archive)
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
