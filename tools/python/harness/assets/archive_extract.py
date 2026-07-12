from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image

from .emi_archive import EmiArchive
from .image_io import build_contact_sheet, palette_preview_image, save_rgba_image
from .indexed_image import (
    decode_indexed_image,
    decode_palette_rows,
    indices_to_rgba_bytes,
    infer_bpp,
    infer_stripped_width,
)


@dataclass(frozen=True)
class ArchiveEntry:
    index: int
    name: str
    type: int
    size: int
    ram_ptr: int
    path: Path | None = None
    payload: bytes | None = None

    def read_bytes(self) -> bytes:
        if self.payload is not None:
            return self.payload
        if self.path is not None:
            return self.path.read_bytes()
        raise RuntimeError(f"entry {self.index} has no byte source")


def load_manifest(archive_dir: Path) -> list[ArchiveEntry]:
    manifest_path = archive_dir / "emi.json"
    payload = json.loads(manifest_path.read_text(encoding="utf-8"))
    raw_entries = payload.get("entries", [])
    if not isinstance(raw_entries, list):
        raise ValueError(f"invalid EMI manifest: {manifest_path}")

    entries: list[ArchiveEntry] = []
    for raw_entry in raw_entries:
        if not isinstance(raw_entry, dict):
            continue
        index = int(raw_entry["index"])
        name = str(raw_entry["name"])
        entry_path = archive_dir / name
        if not entry_path.is_file():
            continue
        entries.append(
            ArchiveEntry(
                index=index,
                name=name,
                type=int(raw_entry["type"]),
                size=int(raw_entry["size"]),
                ram_ptr=int(raw_entry["ram_ptr"]),
                path=entry_path,
            )
        )
    return entries


def load_emi_archive(archive_path: Path) -> list[ArchiveEntry]:
    archive = EmiArchive(archive_path)
    return [
        ArchiveEntry(
            index=entry.index,
            name=entry.default_name,
            type=entry.type_id,
            size=entry.size,
            ram_ptr=entry.load_arg,
            payload=archive.payload(entry.index),
        )
        for entry in archive.entries
    ]


def load_archive_entries(archive_path: Path) -> list[ArchiveEntry]:
    if archive_path.is_dir():
        return load_manifest(archive_path)
    return load_emi_archive(archive_path)


def is_image_entry(entry: ArchiveEntry) -> bool:
    return entry.type == 3 and Path(entry.name).suffix.lower() == ".img"


def is_palette_candidate(entry: ArchiveEntry) -> bool:
    return (
        entry.type == 0
        and entry.size % 32 == 0
        and entry.size <= 4096
        and Path(entry.name).suffix.lower() == ".bin"
    )


def palette_matches_bpp(entry: ArchiveEntry, bpp: int) -> bool:
    if bpp == 4:
        return entry.size <= 512
    if bpp == 8:
        return entry.size > 512 and entry.size % 512 == 0
    raise ValueError(f"unsupported bpp: {bpp}")


def resolve_entries(
    entries: list[ArchiveEntry],
    selectors: list[str] | None,
    *,
    predicate: callable,
    label: str,
) -> list[ArchiveEntry]:
    filtered = [entry for entry in entries if predicate(entry)]
    if not selectors:
        return filtered

    selected: list[ArchiveEntry] = []
    seen: set[int] = set()
    for selector in selectors:
        matches = [
            entry
            for entry in filtered
            if entry.name == selector or str(entry.index) == selector
        ]
        if not matches:
            raise ValueError(f"no {label} entry matches selector: {selector}")
        for entry in matches:
            if entry.index not in seen:
                seen.add(entry.index)
                selected.append(entry)
    return selected


def export_indices(
    image_entry: ArchiveEntry,
    output_dir: Path,
    *,
    bpp_override: int | None,
    width_override: int | None,
    unstrip: bool,
) -> Path:
    raw_image = image_entry.read_bytes()
    bpp = bpp_override if bpp_override is not None else infer_bpp(image_entry.size)
    stripped_width = (
        width_override
        if width_override is not None
        else infer_stripped_width(image_entry.ram_ptr)
    )
    decoded = decode_indexed_image(
        raw_image,
        bpp=bpp,
        stripped_width=stripped_width,
        unstrip=unstrip,
    )

    scale = 17 if bpp == 4 else 1
    rgba = bytearray(decoded.width * decoded.height * 4)
    for pixel_index, palette_index in enumerate(decoded.indices):
        value = palette_index * scale
        base = pixel_index * 4
        rgba[base] = value
        rgba[base + 1] = value
        rgba[base + 2] = value
        rgba[base + 3] = 0 if palette_index == 0 else 255

    return save_rgba_image(
        decoded.width,
        decoded.height,
        bytes(rgba),
        output_dir / f"{Path(image_entry.name).stem}__indices.png",
    )


def export_palette_preview(entry: ArchiveEntry, output_dir: Path, *, bpp: int) -> Path:
    rows = decode_palette_rows(entry.read_bytes(), bpp)
    output_path = output_dir / f"{Path(entry.name).stem}__{bpp}bpp.png"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    palette_preview_image(rows).save(output_path)
    return output_path


def export_pair(
    image_entry: ArchiveEntry,
    palette_entry: ArchiveEntry,
    output_dir: Path,
    *,
    bpp_override: int | None,
    width_override: int | None,
    palette_row: int | None,
    unstrip: bool,
    columns: int,
) -> list[Path]:
    bpp = (
        bpp_override
        if bpp_override is not None
        else infer_bpp(image_entry.size, palette_entry.size)
    )
    stripped_width = (
        width_override
        if width_override is not None
        else infer_stripped_width(image_entry.ram_ptr)
    )
    decoded = decode_indexed_image(
        image_entry.read_bytes(),
        bpp=bpp,
        stripped_width=stripped_width,
        unstrip=unstrip,
    )
    palette_rows = decode_palette_rows(palette_entry.read_bytes(), bpp)
    base_name = f"{Path(image_entry.name).stem}__{Path(palette_entry.name).stem}"
    output_dir.mkdir(parents=True, exist_ok=True)

    if palette_row is not None:
        if palette_row < 0 or palette_row >= len(palette_rows):
            raise ValueError(
                f"palette row out of range for {palette_entry.name}: {palette_row}"
            )
        output_path = output_dir / f"{base_name}__row{palette_row:02d}.png"
        save_rgba_image(
            decoded.width,
            decoded.height,
            indices_to_rgba_bytes(
                decoded.indices,
                decoded.width,
                decoded.height,
                palette_rows[palette_row],
            ),
            output_path,
        )
        return [output_path]

    rendered_rows = [
        Image.frombytes(
            "RGBA",
            (decoded.width, decoded.height),
            indices_to_rgba_bytes(
                decoded.indices,
                decoded.width,
                decoded.height,
                row,
            ),
        )
        for row in palette_rows
    ]
    if len(rendered_rows) == 1:
        output_path = output_dir / f"{base_name}.png"
        rendered_rows[0].save(output_path)
        return [output_path]

    output_path = output_dir / f"{base_name}__candidate_rows.png"
    build_contact_sheet(
        rendered_rows,
        columns=columns,
        labels=[f"row{index:02d}" for index in range(len(rendered_rows))],
    ).save(output_path)
    return [output_path]


def extract_archive(
    archive_path: Path,
    output_dir: Path,
    *,
    image_selectors: list[str] | None = None,
    palette_selectors: list[str] | None = None,
    bpp_override: int | None = None,
    width_override: int | None = None,
    palette_row: int | None = None,
    columns: int = 4,
    unstrip: bool = True,
    emit_indices: bool = False,
    emit_palette_previews: bool = False,
) -> list[Path]:
    entries = load_archive_entries(archive_path)
    image_entries = resolve_entries(
        entries,
        image_selectors,
        predicate=is_image_entry,
        label="image",
    )
    palette_entries = resolve_entries(
        entries,
        palette_selectors,
        predicate=is_palette_candidate,
        label="palette",
    )

    if not image_entries and not emit_palette_previews:
        raise ValueError(f"no image entries found in {archive_path}")

    written: list[Path] = []
    if emit_palette_previews:
        palette_output_dir = output_dir / "palette_previews"
        for palette_entry in palette_entries:
            for candidate_bpp in (4, 8):
                try:
                    written.append(
                        export_palette_preview(
                            palette_entry,
                            palette_output_dir,
                            bpp=candidate_bpp,
                        )
                    )
                except ValueError:
                    continue

    for image_entry in image_entries:
        if emit_indices or not palette_entries:
            written.append(
                export_indices(
                    image_entry,
                    output_dir,
                    bpp_override=bpp_override,
                    width_override=width_override,
                    unstrip=unstrip,
                )
            )

        if not palette_entries:
            continue

        image_bpp = (
            bpp_override if bpp_override is not None else infer_bpp(image_entry.size)
        )
        selected_palettes = (
            palette_entries
            if palette_selectors
            else [
                palette_entry
                for palette_entry in palette_entries
                if palette_matches_bpp(palette_entry, image_bpp)
            ]
        )
        if not selected_palettes:
            selected_palettes = palette_entries

        for palette_entry in selected_palettes:
            written.extend(
                export_pair(
                    image_entry,
                    palette_entry,
                    output_dir,
                    bpp_override=bpp_override,
                    width_override=width_override,
                    palette_row=palette_row,
                    unstrip=unstrip,
                    columns=columns,
                )
            )

    return written


def extract_tree(
    root: Path,
    output_dir: Path,
    *,
    bpp_override: int | None = None,
    palette_row: int | None = None,
    columns: int = 4,
    unstrip: bool = True,
    emit_indices: bool = False,
) -> dict[str, int]:
    archive_paths = sorted(root.rglob("*.EMI"))
    if archive_paths:
        relative_root = root
    else:
        archive_paths = sorted(path.parent for path in root.rglob("emi.json"))
        relative_root = root

    archive_count = 0
    image_count = 0
    written_count = 0
    for archive_path in archive_paths:
        entries = load_archive_entries(archive_path)
        if not any(is_image_entry(entry) for entry in entries):
            continue
        archive_output_dir = output_dir / archive_path.relative_to(relative_root)
        if archive_path.suffix.upper() == ".EMI":
            archive_output_dir = archive_output_dir.with_suffix("")
        written = extract_archive(
            archive_path,
            archive_output_dir,
            bpp_override=bpp_override,
            palette_row=palette_row,
            columns=columns,
            unstrip=unstrip,
            emit_indices=emit_indices,
        )
        archive_count += 1
        image_count += sum(1 for entry in entries if is_image_entry(entry))
        written_count += len(written)

    return {
        "archive_count": archive_count,
        "image_count": image_count,
        "written_count": written_count,
    }
