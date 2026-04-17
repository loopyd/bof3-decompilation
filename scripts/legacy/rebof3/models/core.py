from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class SourceSpec:
    path: Path
    entry_index: int | None = None

    def __iter__(self):
        yield self.path
        yield self.entry_index


@dataclass(frozen=True)
class BinMetadata:
    manifest_path: str
    entry_name: str | None
    entry_index: int | None
    entry_type: int | None
    load_address: int

    def as_dict(self) -> dict[str, object]:
        return {
            "manifest_path": self.manifest_path,
            "entry_name": self.entry_name,
            "entry_index": self.entry_index,
            "entry_type": self.entry_type,
            "load_address": self.load_address,
        }


@dataclass(frozen=True)
class MatchMetrics:
    asm_score: int | None = None
    asm_max_score: int | None = None
    asm_row_count: int | None = None
    asm_score_per_row: float | None = None
    asm_score_per_byte: float | None = None
    objdiff_match_percent: float | None = None
    objdiff_instruction_count: int | None = None
    objdiff_mismatch_count: int | None = None
    semantic_status: str | None = None
    semantic_classified_mismatch_count: int | None = None
    semantic_unclassified_mismatch_count: int | None = None
    semantic_move_zero_sugar_count: int | None = None
    semantic_li_zero_sugar_count: int | None = None
    semantic_branch_zero_sugar_count: int | None = None
    semantic_commutative_swap_count: int | None = None
    semantic_call_target_reloc_count: int | None = None
    semantic_address_materialization_count: int | None = None
    semantic_asm_view_only_noise: bool | None = None

    def as_dict(self) -> dict[str, object | None]:
        return {
            "asm_score": self.asm_score,
            "asm_max_score": self.asm_max_score,
            "asm_row_count": self.asm_row_count,
            "asm_score_per_row": self.asm_score_per_row,
            "asm_score_per_byte": self.asm_score_per_byte,
            "objdiff_match_percent": self.objdiff_match_percent,
            "objdiff_instruction_count": self.objdiff_instruction_count,
            "objdiff_mismatch_count": self.objdiff_mismatch_count,
            "semantic_status": self.semantic_status,
            "semantic_classified_mismatch_count": self.semantic_classified_mismatch_count,
            "semantic_unclassified_mismatch_count": self.semantic_unclassified_mismatch_count,
            "semantic_move_zero_sugar_count": self.semantic_move_zero_sugar_count,
            "semantic_li_zero_sugar_count": self.semantic_li_zero_sugar_count,
            "semantic_branch_zero_sugar_count": self.semantic_branch_zero_sugar_count,
            "semantic_commutative_swap_count": self.semantic_commutative_swap_count,
            "semantic_call_target_reloc_count": self.semantic_call_target_reloc_count,
            "semantic_address_materialization_count": self.semantic_address_materialization_count,
            "semantic_asm_view_only_noise": self.semantic_asm_view_only_noise,
        }
