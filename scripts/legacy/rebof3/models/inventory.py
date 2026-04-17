from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True, slots=True)
class InventoryProgramRow:
    program_slug: str
    program_name: str
    program_path: str
    folder: str | None = None
    source_hint: str | None = None


@dataclass(frozen=True, slots=True)
class InventoryFunctionRow:
    program_slug: str
    entry_address: int
    entry_hex: str
    name: str
    signature: str | None = None
    body_min: int | None = None
    body_max: int | None = None
    comment: str | None = None
    repeatable_comment: str | None = None
    namespace: str | None = None
    name_source: str | None = None
    is_thunk: bool = False
    source_hint: str | None = None


@dataclass(frozen=True, slots=True)
class InventoryArchiveRow:
    archive_id: str
    archive_name: str
    family: str
    emi_path: str


@dataclass(frozen=True, slots=True)
class InventoryEmiEntryRow:
    archive_id: str
    entry_index: int
    size: int
    family: str
    entry_name: str | None = None
    type_id: int | None = None
    load_arg: int | None = None
    first_word: int | None = None
    sha256: str | None = None
    payload_path: str | None = None
    code_candidate: bool = False
    palette_candidate: bool = False
