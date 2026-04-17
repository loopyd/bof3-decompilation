from __future__ import annotations

from ..models import (
    DuplicateGroup,
    DuplicateGroups,
    InventoryProgram,
    InventorySnapshot,
)


def overlay_program_sort_key(program: InventoryProgram) -> tuple[str, int, str]:
    return (
        program.archive_id or "",
        program.entry_index if program.entry_index is not None else -1,
        program.program_id,
    )


def group_exact_duplicates(snapshot: InventorySnapshot) -> DuplicateGroups:
    buckets: dict[str, list[InventoryProgram]] = {}
    for program in snapshot.programs:
        if program.kind != "overlay":
            continue
        buckets.setdefault(program.sha256, []).append(program)

    groups: list[DuplicateGroup] = []
    for sha256, members in sorted(buckets.items()):
        if len(members) < 2:
            continue
        ordered_members = sorted(members, key=overlay_program_sort_key)
        member_ids = [program.program_id for program in ordered_members]
        groups.append(
            DuplicateGroup(
                duplicate_group_key=sha256,
                representative_program_id=member_ids[0],
                member_program_ids=member_ids,
            )
        )
    return DuplicateGroups(groups=groups)
