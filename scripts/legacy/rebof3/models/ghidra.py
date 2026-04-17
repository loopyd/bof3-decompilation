from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True, slots=True)
class GhidraDecompRequest:
    source_text: str
    address_text: str
    project_dir: Path | None = None
    project_name: str = "bof3_decomp"
    program_name: str | None = None
    artifacts_dir: Path | None = None
    base_addr: int | None = None
    loader_mode: str = "auto"
    asm_backend: str = "ghidra"
    emit_spimdisasm: bool = True
    no_m2c: bool = False
    noanalysis: bool = False
    dry_run: bool = False


@dataclass(frozen=True, slots=True)
class GhidraBootstrapRequest:
    noanalysis: bool = False
    no_restore_metadata: bool = False
    restore_metadata_from: Path | None = None
    strict_restore: bool = False
