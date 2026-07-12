from __future__ import annotations

from .models import (
    DuplicateGroups,
    GhidraImportEntry,
    GhidraImportLoader,
    GhidraImportManifest,
    InventoryProgram,
    InventorySnapshot,
)


def build_loader(program: InventoryProgram) -> GhidraImportLoader:
    if program.loader_mode == "psx":
        return GhidraImportLoader(
            loader_mode="psx",
            processor=program.processor,
            compiler=program.compiler,
            loader_name=None,
            loader_args=[],
        )
    assert program.base_addr is not None
    assert program.length is not None
    return GhidraImportLoader(
        loader_mode="raw",
        processor=program.processor,
        compiler=program.compiler,
        loader_name="BinaryLoader",
        loader_args=[
            {"name": "-loader-baseAddr", "value": f"0x{program.base_addr:x}"},
            {"name": "-loader-fileOffset", "value": f"0x{program.file_offset:x}"},
            {"name": "-loader-length", "value": f"0x{program.length:x}"},
            {"name": "-loader-blockName", "value": program.block_name},
        ],
    )


def build_ghidra_manifest(
    snapshot: InventorySnapshot,
    groups: DuplicateGroups | None = None,
    *,
    analyze: bool = True,
) -> GhidraImportManifest:
    representatives = groups.representative_ids() if groups is not None else set()
    grouped_members = groups.members() if groups is not None else set()

    imports: list[GhidraImportEntry] = []
    for program in sorted(snapshot.programs, key=lambda item: item.program_id):
        if program.kind == "overlay" and groups is not None:
            if (
                program.program_id in grouped_members
                and program.program_id not in representatives
            ):
                continue
        imports.append(
            GhidraImportEntry(
                source=program.source_path,
                display=program.program_id,
                payload_path=program.payload_path,
                project_folder_path=program.project_folder_path,
                program_name=program.program_name,
                loader=build_loader(program),
            )
        )
    return GhidraImportManifest(analyze=analyze, imports=imports)
