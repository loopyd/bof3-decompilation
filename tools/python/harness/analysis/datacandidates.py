"""Rank unlifted ``asm`` ranges that look like data rather than functions.

A range that Splat models as ``asm`` but that carries no function shape (no
prologue, no ``jr $ra`` terminator, pointer/numeric/string content) is a boundary
*misclassification*, not a lift candidate.  Lanes repeatedly discover these by
hand and promote them ``asm`` -> ``rodata``/``textbin``; this owner enumerates the
same evidence read-only so the promotion decision starts from a ranked list.

Evidence only: nothing here edits Splat, maps, sources or the index, and a
candidate is never acceptance.  Applying a promotion remains a reviewed
transaction through the matching contract, followed by ``source splat`` and the
index commands.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import load_target_manifests

_PRINTABLE = re.compile(rb"[\x20-\x7e]{4,}")
_JR_RA = 0x03E00008
_JAL = 0x0000000C
_SLTIU = 0x0000000B


def _classify(chunk: bytes, load_address: int, image_end: int) -> dict:
    """Classify one raw range by simple byte-shape evidence."""

    words = [
        int.from_bytes(chunk[index : index + 4], "little")
        for index in range(0, len(chunk) - 3, 4)
    ]
    in_image = 0
    for word in words:
        if word % 4 == 0 and load_address <= word < image_end:
            in_image += 1
    printable = len(b"".join(match.group(0) for match in _PRINTABLE.finditer(chunk)))
    has_return = _JR_RA in words
    opcodes = [word >> 26 for word in words]
    calls = opcodes.count(_JAL)

    if words and in_image == len(words) and len(words) >= 2:
        kind = "pointer_table"
    elif printable and printable >= max(4, len(chunk) // 2):
        kind = "string_block"
    elif words and not has_return and len(set(words)) <= max(2, len(words) // 2):
        kind = "numeric_table"
    elif not has_return and calls == 0:
        kind = "no_function_shape"
    else:
        kind = "unknown"

    return {
        "kind": kind,
        "words": len(words),
        "in_image_pointers": in_image,
        "printable_bytes": printable,
        "has_jr_ra": has_return,
        "jal_count": calls,
        "sltiu_count": opcodes.count(_SLTIU),
        "sample": [f"0x{word:08x}" for word in words[:6]],
    }


def collect_data_candidates(
    root: Path, target_ids: list[str], *, limit: int = 200
) -> dict:
    """Return ranked non-function ``asm`` ranges across the selected targets."""

    manifests = load_target_manifests(root)
    selected = target_ids or sorted(manifests)
    unknown = [target for target in selected if target not in manifests]
    if unknown:
        raise ValueError(f"unknown target(s): {', '.join(sorted(unknown))}")

    candidates: list[dict] = []
    for target in sorted(selected):
        manifest = manifests[target]
        splat_path = root / manifest.splat
        if not splat_path.is_file():
            continue
        projected = parse_splat_text(splat_path.read_text(), manifest.load_address)
        binary_path = root / manifest.binary
        if not binary_path.is_file():
            continue
        image = binary_path.read_bytes()
        image_end = manifest.load_address + len(image)
        for boundary in projected.boundaries:
            # Unlifted ``asm`` rows are the candidate pool: Splat presents every one
            # as a function boundary, and this owner tests which byte ranges do not
            # actually carry function shape.
            if boundary.kind != "asm":
                continue
            start = boundary.file_start
            end = boundary.file_end
            if start is None or end is None:
                continue
            if end - start < 8 or end > len(image):
                continue
            chunk = image[start:end]
            evidence = _classify(chunk, manifest.load_address, image_end)
            if evidence["kind"] == "unknown":
                continue
            candidates.append(
                {
                    "target": target,
                    "name": boundary.name,
                    "file_start": start,
                    "file_end": end,
                    "virtual_start": f"0x{boundary.virtual_start:08x}",
                    "size": end - start,
                    **evidence,
                }
            )

    # Largest evidence-backed ranges first; pointer tables outrank other shapes.
    order = {"pointer_table": 0, "string_block": 1, "numeric_table": 2, "no_function_shape": 3}
    candidates.sort(key=lambda row: (order.get(row["kind"], 9), -row["size"]))
    return {
        "schema": "bof3.data-candidates/v1",
        "targets": len(selected),
        "total": len(candidates),
        "candidates": candidates[:limit],
        "limits": (
            "Byte-shape evidence only: no whole-image proof, no semantic role, no "
            "layout authority. Promotion requires the reviewed matching transaction "
            "plus `source splat` and a successful `analysis index --recover`/`just index`."
        ),
    }


def run(args) -> int:
    """CLI entry point: rank unlifted ``asm`` ranges that look like data."""

    report = collect_data_candidates(
        args.root.resolve(), args.targets, limit=args.limit
    )
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
        return 0
    print(f"data candidates: targets={report['targets']} total={report['total']}")
    for row in report["candidates"]:
        print(
            f"{row['target']}@{row['virtual_start']} {row['name']} "
            f"kind={row['kind']} size={row['size']} "
            f"in_image={row['in_image_pointers']} jr_ra={row['has_jr_ra']}"
        )
    print(report["limits"])
    return 0


def build_parser() -> "argparse.ArgumentParser":
    parser = argparse.ArgumentParser(
        prog="bin/harness analysis data-candidates", description=__doc__
    )
    parser.add_argument("targets", nargs="*", help="target ids (default: all)")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--limit", type=int, default=200)
    add_root_argument(parser)
    add_example_argument(
        parser, "bin/harness analysis data-candidates emi/etc/shop/00 --json"
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv=None) -> int:
    return run_main(build_parser, argv)
