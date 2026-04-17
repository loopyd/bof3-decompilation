from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ..paths import RepoLayout


@dataclass(frozen=True)
class SetupOptions:
    force: bool = False
    include_aspsx_binaries: bool = True
    include_match_tools: bool = True
    include_psyq: bool = True
    include_extract: bool = True
    include_ghidra_plan: bool = True
    psyq_source_root: Path | None = None
    psyq_archive: Path | None = None


@dataclass(frozen=True)
class SetupTask:
    name: str
    description: str


@dataclass(frozen=True)
class SetupContext:
    layout: RepoLayout
    options: SetupOptions
