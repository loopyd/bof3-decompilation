from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class ResolvedProgramCandidate:
    program_path: str
    program_selector: str
    archive_id: str | None = None
    entry_index: int | None = None
    family: str | None = None
    load_arg: int | None = None
    representative_archive_id: str | None = None
    representative_entry_index: int | None = None
    confidence: str | None = None
    reason: str | None = None

    def as_dict(self) -> dict[str, object | None]:
        return {
            "program_path": self.program_path,
            "program_selector": self.program_selector,
            "archive_id": self.archive_id,
            "entry_index": self.entry_index,
            "family": self.family,
            "load_arg": self.load_arg,
            "representative_archive_id": self.representative_archive_id,
            "representative_entry_index": self.representative_entry_index,
            "confidence": self.confidence,
            "reason": self.reason,
        }


@dataclass(frozen=True, slots=True)
class AddressResolution:
    requested_program_path: str
    requested_program_selector: str
    requested_address: int | None
    requested_kind: str
    resolved_kind: str
    primary_program_selector: str | None = None
    candidate_program_selectors: tuple[str, ...] = ()
    region_base: int | None = None
    region_family: str | None = None
    containing_function_entry: int | None = None
    containing_function_name: str | None = None
    notes: tuple[str, ...] = ()
    xref_strategy: str = "no_address"

    def as_dict(self) -> dict[str, object | None]:
        return {
            "requested_program_path": self.requested_program_path,
            "requested_program_selector": self.requested_program_selector,
            "requested_address": self.requested_address,
            "requested_kind": self.requested_kind,
            "resolved_kind": self.resolved_kind,
            "primary_program_selector": self.primary_program_selector,
            "candidate_program_selectors": list(self.candidate_program_selectors),
            "region_base": self.region_base,
            "region_family": self.region_family,
            "containing_function_entry": self.containing_function_entry,
            "containing_function_name": self.containing_function_name,
            "notes": list(self.notes),
            "xref_strategy": self.xref_strategy,
        }
