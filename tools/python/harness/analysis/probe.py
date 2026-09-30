"""Isolated Splat textbin probes; observations never authorize layout edits."""

from __future__ import annotations

import copy
import hashlib
import os
import re
import subprocess
import tempfile
from pathlib import Path

import yaml

from harness.common.directory import open_parent_fd
from harness.domain.layout import parse_splat_text
from harness.io import repo_layout
from harness.toolchain.splat import SplatToolchain

# Only this bounded configuration shape is redirected. Unknown output options
# must not accidentally send a probe into the live tree.
_OPTIONS = {
    "create_c_files",
    "platform",
    "compiler",
    "basename",
    "base_path",
    "target_path",
    "asm_path",
    "src_path",
    "ld_script_path",
    "undefined_funcs_auto_path",
    "undefined_syms_auto_path",
    "cache_path",
    "asset_path",
    "symbol_addrs_path",
}


def _replace_kind(document, offset: int, name: str) -> None:
    hits = []
    for segment in document["segments"]:
        if not isinstance(segment, dict):
            continue
        for row in segment.get("subsegments", []):
            if isinstance(row, list) and len(row) == 3 and row == [offset, "asm", name]:
                hits.append(row)
    if len(hits) != 1:
        raise ValueError(
            "textbin probe requires exactly one plain [offset, asm, name] subsegment"
        )
    hits[0][1] = "textbin"


def _collect_text_entries(
    script: str, directory: Path, name: str, *, require_selected: bool = True
) -> list[str]:
    """Compare actual .text input order, not guessed alignments or linker addresses.

    ``require_selected=False`` is used for a carve's *before* phase, where the
    selected range is still inside a ``bin`` segment and therefore contributes no
    ``.text`` object at all.
    """

    entries = []
    for line in script.splitlines():
        if re.search(r"\((\.text)\);\s*$", line):
            entry = line.strip().replace(directory.as_posix(), "<probe>")
            entry = entry.replace(f"asm/data/{name}.s.o", f"<selected>/{name}.s.o")
            entry = entry.replace(f"asm/{name}.s.o", f"<selected>/{name}.s.o")
            entries.append(entry)
    if not entries:
        raise ValueError("probe linker script has no .text input")
    if require_selected and not any(
        f"<selected>/{name}.s.o" in line for line in entries
    ):
        raise ValueError("probe linker script has no selected .text input")
    return entries


def _load_probe_documents(root, manifest, inputs):
    """Validate the bounded Splat shape and return the loaded before-document.

    Shared by the textbin and carve probes: only supported PSX configuration and
    segment/row shapes are admitted, so a probe can never rewrite an unknown
    directive or leak into the live tree.
    """

    overlay = yaml.safe_load(inputs.text(root / "config/splat.yaml"))
    if overlay != {"options": {"create_c_files": False, "disassemble_all": True}}:
        raise ValueError("unsupported shared Splat overlay for isolated probe")
    document = yaml.safe_load(inputs.text(root / manifest.splat))
    if not isinstance(document, dict) or set(document) - {
        "name",
        "sha1",
        "options",
        "segments",
    }:
        raise ValueError("unsupported top-level Splat probe configuration")
    options = document["options"]
    if not re.fullmatch(r"[A-Za-z_]\w*", str(options.get("basename", "probe"))):
        raise ValueError("unsupported Splat probe basename")
    unknown = set(options) - _OPTIONS
    if unknown or options.get("platform") != "psx":
        raise ValueError(
            f"unsupported Splat probe options: {sorted(unknown)}; requires psx"
        )
    # Refuse output/path-bearing segment extensions rather than attempting to
    # sanitize arbitrary Splat plugins or source generation directives.
    allowed = {"name", "type", "start", "vram", "subsegments"}
    for segment in document["segments"]:
        if isinstance(segment, dict) and (
            set(segment) - allowed or segment.get("type") not in {"code", "bin"}
        ):
            raise ValueError("unsupported segment options for isolated textbin probe")
        segment_rows = (
            segment.get("subsegments", []) if isinstance(segment, dict) else [segment]
        )
        if isinstance(segment, dict) and not re.fullmatch(
            r"[A-Za-z_]\w*", str(segment.get("name", ""))
        ):
            raise ValueError("unsupported segment name for isolated textbin probe")
        for row in segment_rows:
            if not isinstance(row, list) or not row:
                raise ValueError("unsupported isolated probe row")
            if len(row) == 1:
                continue
            if (
                len(row) < 3
                or row[1] not in {"bin", "c", "asm", "textbin"}
                or not isinstance(row[2], str)
                or not re.fullmatch(r"[A-Za-z_]\w*", row[2])
            ):
                raise ValueError("unsupported isolated probe row kind/name")
            if any(
                not isinstance(value, str)
                or not value.startswith(("@source:", "@behavior:"))
                for value in row[3:]
            ):
                raise ValueError("unsupported isolated probe row metadata")
    return document


def _subsegment_rows(document):
    """Yield ``(segment, rows)`` for every subsegment list in the document."""

    for segment in document["segments"]:
        if not isinstance(segment, dict):
            continue
        rows = segment.get("subsegments")
        if isinstance(rows, list):
            yield segment, rows


def carve_rows(start: int, end: int, name: str):
    """Split one ``bin`` range into head/asm/tail rows inside its subsegment list.

    ``start``/``end`` are file offsets of the carve inside a single ``bin`` row.
    Only the containing row is rewritten; every other row and all offsets are
    preserved, so byte layout cannot move.
    """

    if start >= end:
        raise ValueError("carve range must be non-empty")
    if not re.fullmatch(r"[A-Za-z_]\w*", name):
        raise ValueError("carve name must be a C identifier")

    def apply(rows: list) -> bool:
        for index, row in enumerate(rows):
            if not isinstance(row, list) or len(row) < 3 or row[1] != "bin":
                continue
            row_offset = row[0]
            next_offset = rows[index + 1][0] if index + 1 < len(rows) else None
            if next_offset is None or not (row_offset < start and end <= next_offset):
                continue
            replacement = []
            if start > row_offset:
                replacement.append([row_offset, "bin", f"{row[2]}_head"])
            replacement.append([start, "asm", name])
            if end < next_offset:
                replacement.append([end, "bin", f"{row[2]}_tail"])
            rows[index : index + 1] = replacement
            return True
        return False

    return apply


def _segment_start(segment) -> int | None:
    """Return a segment's file offset: dict ``start`` or bare ``[offset, ...]``."""

    if isinstance(segment, dict):
        value = segment.get("start")
        return value if isinstance(value, int) else None
    if isinstance(segment, list) and segment and isinstance(segment[0], int):
        return segment[0]
    return None


def carve_segments(start: int, end: int, name: str, vram: int | None = None):
    """Split a *bare* ``bin`` segment row into head segment + code segment.

    Edge case the row splitter cannot express: a leading blob is written as a
    bare row (``[0, "bin", "header"]``) rather than a segment with subsegments,
    so the code inside it has no row to split.  This regroups the segment list
    into a head ``bin`` segment, a new ``code`` segment carrying exactly one
    ``asm`` subsegment, and an optional trailing ``bin`` segment - all at
    unchanged offsets, so byte layout cannot move.  Splat requires the code
    segment's ``vram``, which is the image base plus the carved offset.
    """

    if start >= end:
        raise ValueError("carve range must be non-empty")
    if not re.fullmatch(r"[A-Za-z_]\w*", name):
        raise ValueError("carve name must be a C identifier")
    if vram is None:
        raise ValueError("carve requires the code segment vram")

    def apply(segments: list) -> bool:
        starts = [_segment_start(segment) for segment in segments]
        for index, segment in enumerate(segments):
            if not isinstance(segment, list) or len(segment) < 3:
                continue
            if segment[1] != "bin" or not isinstance(segment[0], int):
                continue
            next_offset = next(
                (offset for offset in starts[index + 1 :] if offset is not None), None
            )
            if next_offset is None or not (segment[0] < start and end <= next_offset):
                continue
            replacement: list = []
            if start > segment[0]:
                replacement.append([segment[0], "bin", f"{segment[2]}_head"])
            replacement.append(
                {
                    "name": name,
                    "type": "code",
                    "start": start,
                    "vram": vram,
                    "subsegments": [[start, "asm", name]],
                }
            )
            if end < next_offset:
                replacement.append([end, "bin", f"{segment[2]}_tail"])
            segments[index : index + 1] = replacement
            return True
        return False

    return apply


def probe_textbin(root, manifest, boundary, inputs, chunk: bytes) -> dict:
    """Run two splits on copies, retain logs/config/raw bytes and report exact limits."""
    document = _load_probe_documents(root, manifest, inputs)
    options = document["options"]
    before_text = inputs.text(root / manifest.splat)
    after = copy.deepcopy(document)
    _replace_kind(after, boundary.file_start, boundary.name)
    after_text = yaml.safe_dump(after, sort_keys=False)
    projected = parse_splat_text(after_text, manifest.load_address)
    replacement = projected.find_boundary_at(boundary.virtual_start)
    if replacement is None or replacement.is_function:
        raise ValueError("textbin probe did not remove the selected function boundary")
    before = parse_splat_text(before_text, manifest.load_address)
    expected = [b for b in before.boundaries if b != boundary]
    actual = [b for b in projected.boundaries if b != replacement]
    if expected != actual or (
        replacement.file_start,
        replacement.file_end,
        replacement.virtual_start,
        replacement.virtual_end,
    ) != (
        boundary.file_start,
        boundary.file_end,
        boundary.virtual_start,
        boundary.virtual_end,
    ):
        raise ValueError("textbin probe changed another boundary or selected range")
    tool = SplatToolchain(repo_layout(root))
    if not tool.executable.is_file():
        raise ValueError(
            "missing Splat executable; probe does not install dependencies"
        )
    parent, _ = open_parent_fd(root, "out/boundary-probes/reserved", create=True)
    os.close(parent)
    workspace = Path(
        tempfile.mkdtemp(prefix="probe-", dir=root / "out/boundary-probes")
    )
    result = {
        "status": "failed",
        "workspace": str(workspace),
        "runs": [],
        "whole_image_verified": False,
    }
    (workspace / "candidate.yaml").write_text(after_text)
    binary_path = workspace / "original.bin"
    binary_path.write_bytes(inputs.read(root / manifest.binary))
    maps = []
    for ordinal, name in enumerate(before.symbol_map_paths):
        path = workspace / f"symbols-{ordinal}.txt"
        path.write_bytes(inputs.read(root / name))
        maps.append(str(path))
    scripts = []
    normalized_scripts = []
    for label, config in [("before", document), ("after", after)]:
        directory = workspace / label
        directory.mkdir()
        config = copy.deepcopy(config)
        config["options"] = {
            "platform": "psx",
            "compiler": options.get("compiler", "psyq"),
            "basename": options.get("basename", "probe"),
            "base_path": str(directory),
            "target_path": str(binary_path),
            "symbol_addrs_path": maps,
            "asm_path": "asm",
            "data_path": "data",
            "src_path": "src",
            "asset_path": "assets",
            "ld_script_path": "linker.ld",
            "undefined_funcs_auto_path": "undefined_funcs.txt",
            "undefined_syms_auto_path": "undefined_syms.txt",
            "cache_path": ".splache",
            "create_c_files": False,
            "disassemble_all": True,
        }
        config_path = directory / "splat.yaml"
        config_path.write_text(yaml.safe_dump(config, sort_keys=False))
        argv = ["split", "--make-full-disasm-for-code", str(config_path)]
        try:
            run = tool.execute(argv, capture_output=True, text=True, timeout=120)
        except subprocess.TimeoutExpired:
            result["runs"].append({"phase": label, "status": "timeout"})
            return result
        (directory / "stdout.txt").write_text(run.stdout)
        (directory / "stderr.txt").write_text(run.stderr)
        result["runs"].append(
            {
                "phase": label,
                "argv": tool.invocation(argv),
                "returncode": run.returncode,
            }
        )
        if run.returncode:
            return result
        script = (directory / "linker.ld").read_text()
        scripts.append(_collect_text_entries(script, directory, boundary.name))
        normalized_scripts.append(
            script.replace(str(directory), "<probe>").replace(
                f"asm/data/{boundary.name}.s.o", f"asm/{boundary.name}.s.o"
            )
        )
    bins = list((workspace / "after" / "assets").rglob(f"{boundary.name}.textbin.bin"))
    if len(bins) != 1:
        result["reason"] = "expected one extracted textbin asset"
        return result
    extracted = bins[0].read_bytes()
    assembly = workspace / "after" / "asm" / "data" / f"{boundary.name}.s"
    asm_text = assembly.read_text()
    bytes_equal = extracted == chunk
    text_order_equal = scripts[0] == scripts[1]
    emits_text = bool(re.search(r"^\.section \.text(?:,|\s|$)", asm_text, re.MULTILINE))
    linker_equal = normalized_scripts[0] == normalized_scripts[1]
    result.update(
        {
            "status": "supported"
            if bytes_equal and text_order_equal and emits_text and linker_equal
            else "mismatch",
            "asset": str(bins[0]),
            "asset_sha256": hashlib.sha256(extracted).hexdigest(),
            "asset_size": len(extracted),
            "original_bytes_equal": bytes_equal,
            "text_input_order_equal": text_order_equal,
            "linker_script_equal_except_selected_object_path": linker_equal,
            "emits_text": emits_text,
            "reviewed_ranges_unchanged": True,
            "candidate_is_function": replacement.is_function,
            "candidate_config": str(workspace / "candidate.yaml"),
            "limits": "Split and raw-byte verification only: no assembler/linker run, whole-image equality, semantic data proof or permission to apply.",
        }
    )
    return result


def probe_carve(
    root, manifest, inputs, carve_start: int, carve_end: int, name: str
) -> dict:
    """Isolated ``bin`` -> ``asm`` carve probe; evidence only, never applies.

    The Class A blocker: a Splat segment start (or a leading ``bin`` blob) cuts a
    jump-table arm off the function that owns it, so the range is not liftable as
    written.  This probe re-groups the containing ``bin`` row into head/asm/tail
    at unchanged offsets, runs the same two bounded Splat splits as the textbin
    probe, and reports whether the carve is shape-valid and layout-preserving.

    It never authors a C boundary, never runs an assembler/linker, and proves no
    semantics: a ``supported`` result is a candidate for a reviewed layout
    transaction, not permission to apply one.
    """

    document = _load_probe_documents(root, manifest, inputs)
    before_text = inputs.text(root / manifest.splat)
    after = copy.deepcopy(document)
    apply_carve = carve_rows(carve_start, carve_end, name)
    if not any(apply_carve(rows) for _, rows in _subsegment_rows(after)):
        # Splat's code segments carry ``vram``; derive the image base from an
        # existing segment rather than assuming the manifest load address.
        base = manifest.load_address
        for segment in after["segments"]:
            if (
                isinstance(segment, dict)
                and isinstance(segment.get("start"), int)
                and isinstance(segment.get("vram"), int)
            ):
                base = segment["vram"] - segment["start"]
                break
        apply_segments = carve_segments(
            carve_start, carve_end, name, base + carve_start
        )
        if not apply_segments(after["segments"]):
            raise ValueError(
                "carve range is not inside a single bin subsegment or bare bin segment"
            )
    after_text = yaml.safe_dump(after, sort_keys=False)

    before = parse_splat_text(before_text, manifest.load_address)
    projected = parse_splat_text(after_text, manifest.load_address)
    carved_start = manifest.load_address + carve_start
    new_boundaries = [
        boundary
        for boundary in projected.boundaries
        if boundary.file_start == carve_start
        and boundary.kind == "asm"
        and boundary.name == name
    ]
    if len(new_boundaries) != 1:
        raise ValueError("carve did not produce exactly one asm boundary")
    if (
        new_boundaries[0].file_end != carve_end
        or new_boundaries[0].virtual_start != carved_start
    ):
        raise ValueError("carve boundary does not match the requested range")
    expected = [b for b in before.boundaries if b.is_function]
    actual = [b for b in projected.boundaries if b.is_function and b is not new_boundaries[0]]
    if expected != actual:
        raise ValueError("carve changed another function boundary")

    tool = SplatToolchain(repo_layout(root))
    if not tool.executable.is_file():
        raise ValueError("missing Splat executable; probe does not install dependencies")
    parent, _ = open_parent_fd(root, "out/boundary-probes/reserved", create=True)
    os.close(parent)
    workspace = Path(
        tempfile.mkdtemp(prefix="carve-", dir=root / "out/boundary-probes")
    )
    result = {
        "status": "failed",
        "workspace": str(workspace),
        "runs": [],
        "whole_image_verified": False,
        "carve_start": carve_start,
        "carve_end": carve_end,
        "name": name,
        "limits": (
            "Document regrouping plus two Splat splits only: no C boundary authored, "
            "no assembler/linker run, no whole-image equality proof and no permission to apply."
        ),
    }
    (workspace / "candidate.yaml").write_text(after_text)
    binary_path = workspace / "original.bin"
    binary_path.write_bytes(inputs.read(root / manifest.binary))
    maps = []
    for ordinal, path_name in enumerate(before.symbol_map_paths):
        path = workspace / f"symbols-{ordinal}.txt"
        path.write_bytes(inputs.read(root / path_name))
        maps.append(str(path))
    scripts = []
    normalized_scripts = []
    options = document["options"]
    for label, config in [("before", document), ("after", after)]:
        directory = workspace / label
        directory.mkdir()
        config = copy.deepcopy(config)
        config["options"] = {
            "platform": "psx",
            "compiler": options.get("compiler", "psyq"),
            "basename": options.get("basename", "probe"),
            "base_path": str(directory),
            "target_path": str(binary_path),
            "symbol_addrs_path": maps,
            "asm_path": "asm",
            "data_path": "data",
            "src_path": "src",
            "asset_path": "assets",
            "ld_script_path": "linker.ld",
            "undefined_funcs_auto_path": "undefined_funcs.txt",
            "undefined_syms_auto_path": "undefined_syms.txt",
            "cache_path": ".splache",
            "create_c_files": False,
            "disassemble_all": True,
        }
        config_path = directory / "splat.yaml"
        config_path.write_text(yaml.safe_dump(config, sort_keys=False))
        argv = ["split", "--make-full-disasm-for-code", str(config_path)]
        try:
            run = tool.execute(argv, capture_output=True, text=True, timeout=120)
        except subprocess.TimeoutExpired:
            result["runs"].append({"phase": label, "status": "timeout"})
            return result
        (directory / "stdout.txt").write_text(run.stdout)
        (directory / "stderr.txt").write_text(run.stderr)
        result["runs"].append(
            {"phase": label, "argv": tool.invocation(argv), "returncode": run.returncode}
        )
        if run.returncode:
            return result
        script = (directory / "linker.ld").read_text()
        scripts.append(
            _collect_text_entries(
                script,
                directory,
                name,
                require_selected=label == "after",
            )
        )
        normalized_scripts.append(script.replace(str(directory), "<probe>"))

    emitted = list((workspace / "after" / "asm").rglob(f"{name}.s"))
    emits_text = bool(
        emitted
        and re.search(
            r"^\.section \.text(?:,|\s|$)", emitted[0].read_text(), re.MULTILINE
        )
    )
    # A carve necessarily *adds* a .text object; the criterion is that every
    # pre-existing .text input survives in the same relative order.
    before_entries = scripts[0]
    after_entries = [
        entry for entry in scripts[1] if "<selected>" not in entry
    ]
    order_preserved = before_entries == after_entries
    result.update(
        {
            "status": "supported"
            if order_preserved and emits_text
            else "mismatch",
            "text_input_order_preserved": order_preserved,
            "emits_text": emits_text,
            "emitted_assembly": str(emitted[0]) if emitted else None,
            "reviewed_ranges_unchanged": True,
            "candidate_config": str(workspace / "candidate.yaml"),
            "limits": (
                "Document regrouping plus two Splat splits only: no C boundary authored, "
                "no assembler/linker run, no whole-image equality proof and no permission to apply."
            ),
        }
    )
    return result
