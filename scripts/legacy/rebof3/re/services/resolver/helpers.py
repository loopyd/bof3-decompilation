from __future__ import annotations

MAX_VISIBLE_CANDIDATES = 8


def is_runtime_psx_address(address: int) -> bool:
    return 0x80000000 <= address < 0x80400000


def representative_key(
    *,
    archive_id: str | None,
    entry_index: int | None,
    representative_archive_id: str | None,
    representative_entry_index: int | None,
) -> tuple[str | None, int | None]:
    return (
        representative_archive_id or archive_id,
        representative_entry_index
        if representative_entry_index is not None
        else entry_index,
    )


def rank_candidates(
    candidates,
    *,
    requested_selector: str,
    requested_family: str | None,
    requested_representative: tuple[str | None, int | None] | None,
):
    def sort_key(candidate) -> tuple[int, int, int, int, str]:
        same_selector = 0 if candidate.program_selector == requested_selector else 1
        same_family = (
            0 if requested_family and candidate.family == requested_family else 1
        )
        same_representative = (
            0
            if requested_representative is not None
            and representative_key(
                archive_id=candidate.archive_id,
                entry_index=candidate.entry_index,
                representative_archive_id=candidate.representative_archive_id,
                representative_entry_index=candidate.representative_entry_index,
            )
            == requested_representative
            else 1
        )
        representative_candidate = (
            0
            if candidate.archive_id == candidate.representative_archive_id
            and candidate.entry_index == candidate.representative_entry_index
            else 1
        )
        return (
            same_selector,
            same_family,
            same_representative,
            representative_candidate,
            candidate.program_selector,
        )

    return sorted(candidates, key=sort_key)


def visible_candidate_selectors(
    candidates,
    *,
    requested_family: str | None,
) -> tuple[tuple[str, ...], tuple[str, ...]]:
    ranked = list(candidates)
    notes: list[str] = []
    visible = ranked
    if requested_family:
        same_family = [
            candidate for candidate in ranked if candidate.family == requested_family
        ]
        if same_family:
            visible = same_family
            notes.append(
                f"prioritized {len(same_family)} same-family candidates out of {len(ranked)} total"
            )
    if len(visible) > MAX_VISIBLE_CANDIDATES:
        notes.append(
            f"candidate list truncated to first {MAX_VISIBLE_CANDIDATES} selectors"
        )
        visible = visible[:MAX_VISIBLE_CANDIDATES]
    return tuple(candidate.program_selector for candidate in visible), tuple(notes)
