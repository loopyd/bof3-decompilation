from __future__ import annotations

from pathlib import Path

from rebof3.build.raw_module import build_raw_image, function_address_from_object


def test_function_address_from_object_name() -> None:
    assert function_address_from_object(Path("func_801d3844.c.obj")) == 0x801D3844


def test_build_raw_image_places_object_text_by_function_address(
    tmp_path: Path,
) -> None:
    objcopy = tmp_path / "objcopy"
    objcopy.write_text(
        "#!/bin/sh\ncp \"$5\" \"$6\"\n",
        encoding="utf-8",
    )
    objcopy.chmod(0o755)
    first = tmp_path / "func_801d0c04.c.obj"
    second = tmp_path / "func_801d0c10.c.obj"
    first.write_bytes(b"abcd")
    second.write_bytes(b"XYZ")

    image, placements = build_raw_image(
        objcopy=objcopy,
        objects=[second, first],
        base_address=0x801D0C00,
        output_size=0x20,
    )

    assert image[4:8] == b"abcd"
    assert image[0x10:0x13] == b"XYZ"
    assert len(image) == 0x20
    assert [placement.address for placement in placements] == [0x801D0C04, 0x801D0C10]


def test_build_raw_image_can_truncate_overlapping_function_text(
    tmp_path: Path,
) -> None:
    objcopy = tmp_path / "objcopy"
    objcopy.write_text(
        "#!/bin/sh\ncp \"$5\" \"$6\"\n",
        encoding="utf-8",
    )
    objcopy.chmod(0o755)
    first = tmp_path / "func_801d0c00.c.obj"
    second = tmp_path / "func_801d0c04.c.obj"
    first.write_bytes(b"abcdef")
    second.write_bytes(b"XYZ")

    image, placements = build_raw_image(
        objcopy=objcopy,
        objects=[first, second],
        base_address=0x801D0C00,
        output_size=0x10,
        truncate_overlaps=True,
    )

    assert image[:7] == b"abcdXYZ"
    assert placements[0].size == 4
    assert placements[0].original_size == 6
    assert placements[0].truncated
