from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class FunctionTarget:
    program_selector: str
    entry_text: str
    entry_hex: str
    entry_value: int
    program_path: str
    program_slug: str


@dataclass(frozen=True)
class WorkspaceRef:
    root: Path
    dir_path: Path
    json_path: Path
