from __future__ import annotations

from pathlib import Path

from ..models import SetupContext


def detect_disk_inputs(context: SetupContext) -> list[Path]:
    matches: list[Path] = []
    for pattern in ("*.cue", "*.bin", "*.iso"):
        matches.extend(sorted(context.layout.disc_dir.glob(pattern)))
    return matches
