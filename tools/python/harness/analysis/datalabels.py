"""Flag ranges that Splat still labels ``asm`` although their bytes are data.

The target contract requires data to be labelled as data: pointer tables, string
runs, count words and numeric blobs belong to ``rodata``/``textbin``, never to an
``asm`` subsegment that claims instructions. The classification evidence already
exists in :mod:`harness.analysis.datacandidates`; this owner turns the decisive part
of it into a check so the rule is verifiable instead of remembered.

Evidence only: nothing here edits Splat, maps, sources or the index. Applying a
promotion remains a reviewed transaction, followed by ``source splat`` for the
target and the usual gates.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from harness.domain.layout import parse_splat_text
from harness.domain.manifests import load_target_manifests
from harness.common.cli import add_example_argument, add_root_argument, run_main

#: Kinds that are decisive on their own: every word is an in-image pointer, or the
#: range is mostly printable text.
STRONG_KINDS = ("pointer_table", "string_block")

#: Kinds that are decisive only with the repetition evidence below, or else weak.
DECISIVE_KINDS = ("numeric_table",)

#: ``no_function_shape`` means only that no prologue or return was seen, which also
#: describes straight-line code fragments; it is reported separately, never counted.
WEAK_KINDS = ("no_function_shape",)


WINDOW_WORDS = 8


def _window_kind(
    words: list[int], chunk: bytes, load: int, image_end: int
) -> str | None:
    """Classify one window; returns a decisive kind or ``None``.

    Windows are used instead of whole subsegments because a single ``asm`` row can
    span a pointer table plus unrelated bytes, and whole-range ratios then average
    the evidence away. A pointer window needs a clear majority of in-image pointers
    so a table followed by other data still registers.
    """

    if not words:
        return None
    pointers = sum(1 for word in words if word % 4 == 0 and load <= word < image_end)
    if pointers >= max(4, (len(words) * 3) // 4):
        return "pointer_table"
    printable = len(re.findall(rb"[\x20-\x7e]", chunk))
    if printable >= max(24, len(chunk) // 2):
        return "string_block"
    unique = len(set(words))
    zeros = sum(1 for word in words if word == 0)
    if unique <= 2 or zeros >= len(words) // 2:
        return "numeric_table"
    return None


def _merge(hits: list[dict]) -> list[dict]:
    """Merge adjacent windows of the same kind into one reported range."""

    merged: list[dict] = []
    for hit in hits:
        if (
            merged
            and merged[-1]["kind"] == hit["kind"]
            and merged[-1]["end"] == hit["start"]
        ):
            merged[-1]["end"] = hit["end"]
            merged[-1]["windows"] += 1
            merged[-1]["pointers"] += hit["pointers"]
            merged[-1]["words"] += hit["words"]
            continue
        merged.append(dict(hit))
    for row in merged:
        row["size"] = row["end"] - row["start"]
    # One short window is often a code sequence full of address constants; require a
    # second window or a window made entirely of pointers before reporting.
    return [
        row
        for row in merged
        if row["windows"] > 1 or row["pointers"] == row["words"]
    ]


def collect_label_findings(root: Path, target_ids: list[str]) -> dict:
    """Return ``asm`` sub-ranges whose bytes are decisively data."""

    manifests = load_target_manifests(root)
    selected = target_ids or sorted(manifests)
    unknown = [target for target in selected if target not in manifests]
    if unknown:
        raise ValueError(f"unknown target(s): {', '.join(sorted(unknown))}")
    findings: list[dict] = []
    scanned_asm = 0
    scanned_windows = 0
    for target in sorted(selected):
        manifest = manifests[target]
        splat = root / manifest.splat
        image_path = root / manifest.binary
        if not splat.is_file() or not image_path.is_file():
            continue
        layout = parse_splat_text(splat.read_text(), manifest.load_address)
        image = image_path.read_bytes()
        image_end = manifest.load_address + len(image)
        starts = [item.file_start for item in layout.boundaries]
        for boundary in layout.boundaries:
            if boundary.kind != "asm" or boundary.file_start is None:
                continue
            start = boundary.file_start
            end = boundary.file_end
            if end is None:
                following = [
                    value for value in starts if value is not None and value > start
                ]
                end = min(following) if following else len(image)
            if end > len(image) or end - start < WINDOW_WORDS * 4:
                continue
            scanned_asm += 1
            chunk = image[start:end]
            hits: list[dict] = []
            step = WINDOW_WORDS * 4
            for offset in range(0, len(chunk) - step + 1, step):
                window = chunk[offset : offset + step]
                words = [
                    int.from_bytes(window[index : index + 4], "little")
                    for index in range(0, step, 4)
                ]
                scanned_windows += 1
                kind = _window_kind(words, window, manifest.load_address, image_end)
                if kind is None:
                    continue
                pointers = sum(
                    1
                    for word in words
                    if word % 4 == 0 and manifest.load_address <= word < image_end
                )
                hits.append(
                    {
                        "target": target,
                        "name": boundary.name,
                        "kind": kind,
                        "start": start + offset,
                        "end": start + offset + step,
                        "virtual_start": f"0x{boundary.virtual_start + offset:08x}",
                        "pointers": pointers,
                        "words": len(words),
                        "windows": 1,
                        "sample": [f"0x{word:08x}" for word in words[:4]],
                    }
                )
            for row in _merge(hits):
                row["size"] = row["end"] - row["start"]
                findings.append(row)
    findings.sort(key=lambda row: (row["target"], row["start"]))
    return {
        "schema": "harness.analysis.data-labels/v1",
        "targets": len(selected),
        "findings": findings,
        "count": len(findings),
        "asm_scanned": scanned_asm,
        "windows_scanned": scanned_windows,
        "weak_count": 0,
        "weak": [],
    }


def run(args: argparse.Namespace) -> int:
    report = collect_label_findings(args.root.resolve(), args.targets)
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        for row in report["findings"]:
            print(
                f"{row['target']} {row['virtual_start']} {row['name']} "
                f"{row['kind']} size={row['size']} "
                f"pointers={row['pointers']}/{row['words']} "
                f"sample={' '.join(row['sample'][:3])}"
            )
        print(
            f"data-labelled-asm: {report['count']} data ranges, "
            f"{report['asm_scanned']} asm ranges and "
            f"{report['windows_scanned']} windows scanned"
        )
    if args.check and report["count"]:
        return 1
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/harness analysis data-labels",
        description=(
            "flag asm subsegments whose bytes are decisively data; "
            "evidence only, nothing is edited"
        ),
    )
    add_root_argument(parser)
    add_example_argument(
        parser,
        "bin/harness analysis data-labels emi/world00/area030/04 --check",
    )
    parser.add_argument(
        "targets", nargs="*", help="target ids; default is every target"
    )
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--check",
        action="store_true",
        help="exit 1 when any decisive finding remains, for use as a gate",
    )
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
