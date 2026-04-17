from __future__ import annotations

from pathlib import Path

from ....inventory.db.connection import inventory_db
from ....inventory.repositories.overlay_resolution import OverlayResolutionRepository
from ....inventory.repositories.programs import ProgramRepository
from ....models.address_resolution import AddressResolution
from .helpers import (
    is_runtime_psx_address,
    rank_candidates,
    representative_key,
    visible_candidate_selectors,
)


def resolve_address_context(
    *,
    db_path: Path,
    program_path: str,
    address: int | None,
    kind: str,
) -> AddressResolution:
    with inventory_db(db_path) as connection:
        programs = ProgramRepository(connection)
        overlays = OverlayResolutionRepository(connection)
        requested_selector = programs.resolve_program_selector(
            program_path=program_path
        )

        if address is None:
            return AddressResolution(
                requested_program_path=program_path,
                requested_program_selector=requested_selector,
                requested_address=None,
                requested_kind=kind,
                resolved_kind="no_address",
                primary_program_selector=requested_selector,
                candidate_program_selectors=(requested_selector,),
                notes=("row has no address",),
                xref_strategy="no_address",
            )

        overlay_row = overlays.get_program_overlay_row(program_path)
        containing_function = overlays.find_containing_function(
            program_path=program_path,
            address=address,
        )
        label_hits = overlays.list_entry_label_hits(address)

        region_base = (
            None
            if overlay_row is None or overlay_row["load_arg"] is None
            else int(overlay_row["load_arg"])
        )
        region_family = (
            None
            if overlay_row is None or overlay_row["family"] is None
            else str(overlay_row["family"])
        )
        requested_representative = (
            None
            if overlay_row is None
            else representative_key(
                archive_id=(
                    None
                    if overlay_row["archive_id"] is None
                    else str(overlay_row["archive_id"])
                ),
                entry_index=(
                    None
                    if overlay_row["entry_index"] is None
                    else int(overlay_row["entry_index"])
                ),
                representative_archive_id=(
                    None
                    if overlay_row["representative_archive_id"] is None
                    else str(overlay_row["representative_archive_id"])
                ),
                representative_entry_index=(
                    None
                    if overlay_row["representative_entry_index"] is None
                    else int(overlay_row["representative_entry_index"])
                ),
            )
        )

        if containing_function is not None:
            entry_address = int(containing_function["entry_address"])
            if address != entry_address:
                return AddressResolution(
                    requested_program_path=program_path,
                    requested_program_selector=requested_selector,
                    requested_address=address,
                    requested_kind=kind,
                    resolved_kind="internal_label",
                    primary_program_selector=requested_selector,
                    candidate_program_selectors=(requested_selector,),
                    region_base=region_base,
                    region_family=region_family,
                    containing_function_entry=entry_address,
                    containing_function_name=str(containing_function["name"] or ""),
                    notes=("address resolves inside containing function",),
                    xref_strategy="containing_function_then_exact_refs",
                )
            return AddressResolution(
                requested_program_path=program_path,
                requested_program_selector=requested_selector,
                requested_address=address,
                requested_kind=kind,
                resolved_kind="in_program",
                primary_program_selector=requested_selector,
                candidate_program_selectors=(requested_selector,),
                region_base=region_base,
                region_family=region_family,
                containing_function_entry=entry_address,
                containing_function_name=str(containing_function["name"] or ""),
                notes=("address matches known program function",),
                xref_strategy="direct_program_xrefs",
            )

        if label_hits:
            notes = ["exact overlay entry-label hit"]
            if region_base is not None:
                notes.append("requested program has matching overlay region")
            return AddressResolution(
                requested_program_path=program_path,
                requested_program_selector=requested_selector,
                requested_address=address,
                requested_kind=kind,
                resolved_kind="internal_label",
                primary_program_selector=requested_selector,
                candidate_program_selectors=(requested_selector,),
                region_base=region_base,
                region_family=region_family,
                notes=tuple(notes),
                xref_strategy="containing_function_then_exact_refs",
            )

        if region_base is not None:
            ranked_candidates = rank_candidates(
                overlays.list_overlay_candidates_by_load_arg(region_base),
                requested_selector=requested_selector,
                requested_family=region_family,
                requested_representative=requested_representative,
            )
            candidate_selectors, candidate_notes = visible_candidate_selectors(
                ranked_candidates,
                requested_family=region_family,
            )
            if len(ranked_candidates) > 1 and is_runtime_psx_address(address):
                return AddressResolution(
                    requested_program_path=program_path,
                    requested_program_selector=requested_selector,
                    requested_address=address,
                    requested_kind=kind,
                    resolved_kind="inventory_shared_region_candidate",
                    primary_program_selector=(
                        candidate_selectors[0]
                        if len(candidate_selectors) == 1
                        else None
                    ),
                    candidate_program_selectors=candidate_selectors,
                    region_base=region_base,
                    region_family=region_family,
                    notes=(
                        "address falls in reused overlay region",
                        (
                            "multiple overlay candidates share load region"
                            f" 0x{region_base:08x}"
                        ),
                        *candidate_notes,
                    ),
                    xref_strategy="ranked_overlay_candidates",
                )

        return AddressResolution(
            requested_program_path=program_path,
            requested_program_selector=requested_selector,
            requested_address=address,
            requested_kind=kind,
            resolved_kind="runtime_only_candidate",
            primary_program_selector=requested_selector,
            candidate_program_selectors=(requested_selector,),
            region_base=region_base,
            region_family=region_family,
            notes=("no inventory-backed owner resolution found",),
            xref_strategy="runtime_neighborhood",
        )
