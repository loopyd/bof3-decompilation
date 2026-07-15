from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ..io import RepoLayout


@dataclass(frozen=True)
class SetupOptions:
    force: bool = False
    include_psyq: bool = True
    include_extract: bool = True
    include_ghidra_plan: bool = False
    psyq_version: str | None = None
    psyq_source_root: Path | None = None
    psyq_archive: Path | None = None
    disc_archive: Path | None = None


@dataclass(frozen=True)
class SetupTask:
    name: str
    description: str


@dataclass(frozen=True)
class SetupContext:
    layout: RepoLayout
    options: SetupOptions
