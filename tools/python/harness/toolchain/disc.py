from .base import Toolchain
from .helpers import (
    download_file,
    find_matching_files,
    require_path_under,
    unique_paths,
)
from .releases import (
DEFAULT_DISC_DIR = REPO_ROOT / "inputs" / "external"
DEFAULT_PRIVATE_ASSETS_ROOT = DEFAULT_DISC_DIR / "private-assets"
AUTO_DISCOVERY_ARCHIVES = (DEFAULT_DISC_DIR / "BreathOfFireIIIv1.1.7z",)
            require_path_under(
                explicit_archive, REPO_ROOT / "inputs", label="BOF3 archive"
            )
    for candidate in unique_paths(candidates):
        matches = find_matching_files(candidate, archive_path_looks_valid)
        return download_file(archive_url, archive_store / archive_name, force=force)
        staged_paths=(cue_path, *bin_paths),


class DiscToolchain(Toolchain):
    label = "disc media"

    def __init__(self, root: Path) -> None:
        self.root = root
        self.cue_path: Path | None = None

    def install(self, *, force: bool = False) -> str:
        disc_root = self.root / "inputs" / "external"
        try:
            self.cue_path, tracks = find_disc_set(disc_root)
        except FileNotFoundError:
            result = import_bof3_disc(dest=disc_root, force=force)
            self.cue_path, tracks = result.cue_path, list(result.staged_paths[1:])
        return f"{self.cue_path.name}, {len(tracks)} tracks"

    def verify(self) -> str:
        cue, tracks = find_disc_set(self.root / "inputs" / "external")
        self.cue_path = cue
        return f"{cue.name}, {len(tracks)} tracks"
