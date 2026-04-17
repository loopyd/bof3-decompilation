from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True, slots=True)
class PlannedProbeResult:
    status: str
    reason: str | None = None
    memory_map_note: str | None = None
    payload: dict[str, object] | None = None

    def as_dict(self) -> dict[str, object | None]:
        return {
            "status": self.status,
            "reason": self.reason,
            "memory_map_note": self.memory_map_note,
            "payload": dict(self.payload or {}),
        }


@dataclass(frozen=True, slots=True)
class MetadataTypeNormalization:
    original: str | None
    normalized: str | None
    status: str
    reason: str | None = None
    is_pseudo_type: bool = False

    def as_dict(self) -> dict[str, object | None]:
        return {
            "original": self.original,
            "normalized": self.normalized,
            "status": self.status,
            "reason": self.reason,
            "is_pseudo_type": self.is_pseudo_type,
        }


@dataclass(frozen=True, slots=True)
class MetadataSyncRowPlan:
    row_key: str
    kind: str
    program_path: str | None
    phase: str
    classification: str
    normalization: MetadataTypeNormalization
    row: dict[str, object]
    dependency_names: tuple[str, ...] = ()
    blocked_reason: str | None = None

    def as_dict(self) -> dict[str, object]:
        return {
            "row_key": self.row_key,
            "kind": self.kind,
            "program_path": self.program_path,
            "phase": self.phase,
            "classification": self.classification,
            "normalization": self.normalization.as_dict(),
            "row": dict(self.row),
            "dependency_names": list(self.dependency_names),
            "blocked_reason": self.blocked_reason,
        }


@dataclass(frozen=True, slots=True)
class MetadataSyncBatch:
    phase: str
    rows: tuple[MetadataSyncRowPlan, ...]

    def as_dict(self) -> dict[str, object]:
        return {
            "phase": self.phase,
            "row_count": len(self.rows),
            "rows": [row.as_dict() for row in self.rows],
        }


@dataclass(frozen=True, slots=True)
class MetadataSyncPlan:
    mode: str
    db_path: str
    selector_scope: tuple[str, ...]
    program_selectors: dict[str, str]
    requested_kind: str
    total_rows: int
    row_plans: tuple[MetadataSyncRowPlan, ...]
    batches: tuple[MetadataSyncBatch, ...]

    def as_dict(self) -> dict[str, object]:
        counts: dict[str, int] = {}
        for row in self.row_plans:
            counts[row.classification] = counts.get(row.classification, 0) + 1
        return {
            "mode": self.mode,
            "db_path": self.db_path,
            "selector_scope": list(self.selector_scope),
            "program_selectors": dict(self.program_selectors),
            "requested_kind": self.requested_kind,
            "total_rows": self.total_rows,
            "classification_counts": counts,
            "batches": [batch.as_dict() for batch in self.batches],
            "rows": [row.as_dict() for row in self.row_plans],
        }


@dataclass(frozen=True, slots=True)
class MetadataSyncToRequest:
    db_path: Path
    mode: str
    owner: str | None = None
    selectors: tuple[str, ...] = ()
    kind: str = "all"
    project_dir: Path = Path("tmp/bof3_ghidra/main")
    project_name: str = "bof3_main"
    output_path: Path | None = None
    log_path: Path | None = None


@dataclass(frozen=True, slots=True)
class MetadataSyncFromRequest:
    db_path: Path
    mode: str
    owner: str | None = None
    selectors: tuple[str, ...] = ()
    kind: str = "all"
    project_dir: Path = Path("tmp/bof3_ghidra/main")
    project_name: str = "bof3_main"
    include_default: bool = True
    user_defined_only: bool = False
    output_path: Path | None = None
    log_path: Path | None = None
    input_path: Path | None = None
