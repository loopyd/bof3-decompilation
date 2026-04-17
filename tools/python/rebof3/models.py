from __future__ import annotations

from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class InventoryProgram:
    program_id: str
    kind: str
    source_path: str
    payload_path: str
    project_folder_path: str
    program_name: str
    loader_mode: str
    processor: str
    compiler: str
    base_addr: int | None
    file_offset: int
    length: int | None
    block_name: str
    size: int
    sha256: str
    family: str | None = None
    archive_id: str | None = None
    entry_index: int | None = None

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "InventoryProgram":
        return cls(
            program_id=str(payload["program_id"]),
            kind=str(payload["kind"]),
            source_path=str(payload["source_path"]),
            payload_path=str(payload["payload_path"]),
            project_folder_path=str(payload["project_folder_path"]),
            program_name=str(payload["program_name"]),
            loader_mode=str(payload["loader_mode"]),
            processor=str(payload["processor"]),
            compiler=str(payload["compiler"]),
            base_addr=(
                None if payload.get("base_addr") is None else int(payload["base_addr"])
            ),
            file_offset=int(payload["file_offset"]),
            length=None if payload.get("length") is None else int(payload["length"]),
            block_name=str(payload["block_name"]),
            size=int(payload["size"]),
            sha256=str(payload["sha256"]),
            family=None if payload.get("family") is None else str(payload["family"]),
            archive_id=(
                None
                if payload.get("archive_id") is None
                else str(payload["archive_id"])
            ),
            entry_index=(
                None
                if payload.get("entry_index") is None
                else int(payload["entry_index"])
            ),
        )

    def to_dict(self) -> dict[str, Any]:
        return {
            "archive_id": self.archive_id,
            "base_addr": self.base_addr,
            "block_name": self.block_name,
            "compiler": self.compiler,
            "entry_index": self.entry_index,
            "family": self.family,
            "file_offset": self.file_offset,
            "kind": self.kind,
            "length": self.length,
            "loader_mode": self.loader_mode,
            "payload_path": self.payload_path,
            "processor": self.processor,
            "program_id": self.program_id,
            "program_name": self.program_name,
            "project_folder_path": self.project_folder_path,
            "sha256": self.sha256,
            "size": self.size,
            "source_path": self.source_path,
        }


@dataclass(frozen=True)
class InventorySnapshot:
    programs: list[InventoryProgram]

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "InventorySnapshot":
        programs_payload = payload.get("programs", [])
        if not isinstance(programs_payload, list):
            raise ValueError("inventory payload must contain a programs array")
        return cls(
            programs=[
                InventoryProgram.from_dict(program_payload)
                for program_payload in programs_payload
            ]
        )

    def to_dict(self) -> dict[str, Any]:
        return {
            "programs": [program.to_dict() for program in self.programs],
            "schema": "rebof3-simple.inventory/v1",
        }


@dataclass(frozen=True)
class DuplicateGroup:
    duplicate_group_key: str
    representative_program_id: str
    member_program_ids: list[str]

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "DuplicateGroup":
        members = payload.get("member_program_ids", [])
        if not isinstance(members, list):
            raise ValueError("duplicate group members must be a list")
        return cls(
            duplicate_group_key=str(payload["duplicate_group_key"]),
            representative_program_id=str(payload["representative_program_id"]),
            member_program_ids=[str(member) for member in members],
        )

    def to_dict(self) -> dict[str, Any]:
        return {
            "duplicate_group_key": self.duplicate_group_key,
            "member_program_ids": self.member_program_ids,
            "representative_program_id": self.representative_program_id,
        }


@dataclass(frozen=True)
class DuplicateGroups:
    groups: list[DuplicateGroup]

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> "DuplicateGroups":
        groups_payload = payload.get("groups", [])
        if not isinstance(groups_payload, list):
            raise ValueError("groups payload must contain a groups array")
        return cls(groups=[DuplicateGroup.from_dict(group) for group in groups_payload])

    def to_dict(self) -> dict[str, Any]:
        return {
            "groups": [group.to_dict() for group in self.groups],
            "schema": "rebof3-simple.duplicates/v1",
        }

    def representative_ids(self) -> set[str]:
        return {group.representative_program_id for group in self.groups}

    def members(self) -> set[str]:
        member_ids: set[str] = set()
        for group in self.groups:
            member_ids.update(group.member_program_ids)
        return member_ids


@dataclass(frozen=True)
class GhidraImportLoader:
    loader_mode: str
    processor: str
    compiler: str
    loader_name: str | None
    loader_args: list[dict[str, str]]

    def to_dict(self) -> dict[str, Any]:
        return {
            "compiler": self.compiler,
            "loader_args": self.loader_args,
            "loader_mode": self.loader_mode,
            "loader_name": self.loader_name,
            "processor": self.processor,
        }


@dataclass(frozen=True)
class GhidraImportEntry:
    source: str
    display: str
    payload_path: str
    project_folder_path: str
    program_name: str
    loader: GhidraImportLoader

    def to_dict(self) -> dict[str, Any]:
        return {
            "display": self.display,
            "loader": self.loader.to_dict(),
            "payload_path": self.payload_path,
            "program_name": self.program_name,
            "project_folder_path": self.project_folder_path,
            "source": self.source,
        }


@dataclass(frozen=True)
class GhidraImportManifest:
    analyze: bool
    imports: list[GhidraImportEntry]

    def to_dict(self) -> dict[str, Any]:
        return {
            "analyze": self.analyze,
            "imports": [entry.to_dict() for entry in self.imports],
            "schema": "rebof3-simple.ghidra-import/v1",
        }
