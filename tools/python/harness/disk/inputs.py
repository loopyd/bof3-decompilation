from __future__ import annotations

from pathlib import Path


def detect_disk_inputs(disc_dir: Path) -> list[Path]:
    matches: list[Path] = []
    for pattern in ("*.cue", "*.bin", "*.iso"):
        matches.extend(sorted(disc_dir.glob(pattern)))
    return matches


def resolve_disc_input_path(disc_dir: Path) -> Path | None:
    matches = detect_disk_inputs(disc_dir)
    for suffix in (".cue", ".iso", ".bin"):
        for candidate in matches:
            if candidate.suffix.lower() == suffix:
                return candidate
    return None
