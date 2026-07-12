from __future__ import annotations

from pathlib import Path

from .archive_extract import export_indices
from .image_io import build_contact_sheet
from .archive_extract import ArchiveEntry


def write_index_preview(
    payload: bytes,
    size: int,
    load_arg: int,
    output_path: Path,
) -> Path:
    entry = ArchiveEntry(
        index=0,
        name=output_path.stem,
        type=3,
        size=size,
        ram_ptr=load_arg,
        payload=payload,
    )
    return export_indices(
        entry,
        output_path.parent,
        bpp_override=None,
        width_override=None,
        unstrip=True,
    )


def write_candidate_sheet(images: list, output_path: Path, *, columns: int) -> Path:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    build_contact_sheet(images, columns=columns).save(output_path)
    return output_path
