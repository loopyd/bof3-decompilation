"""Data contracts for domain-owned patch targets and validated patch series."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Callable


@dataclass(frozen=True)
class Target:
    name: str
    root: Path
    source: Path
    revision: str
    inspect: Callable[[set[str]], dict]
    prepare: Callable[[dict], None]
    patch_order: tuple[str, ...] = ()


@dataclass(frozen=True)
class Patch:
    name: str
    path: Path
    sha256: str
    files: tuple[str, ...]
    data: bytes


@dataclass
class Series:
    target: Target
    patches: list[Patch]
    versions: list[dict[str, bytes | None]]
    groups: list[list[int]]
    counts: list[int]
    observed: dict[str, bytes | None]
    modes: dict[str, int]
    evidence: dict
