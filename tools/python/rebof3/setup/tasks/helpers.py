from __future__ import annotations

from pathlib import Path

from ..models import SetupContext


def detect_disk_inputs(context: SetupContext) -> list[Path]:
    matches: list[Path] = []
    for pattern in ("*.cue", "*.bin", "*.iso"):
        matches.extend(sorted(context.layout.disc_dir.glob(pattern)))
    return matches


def resolve_disc_input_path(context: SetupContext) -> Path | None:
    matches = detect_disk_inputs(context)
    for suffix in (".cue", ".iso", ".bin"):
        for candidate in matches:
            if candidate.suffix.lower() == suffix:
                return candidate
    return None
