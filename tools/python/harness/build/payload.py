"""Assemble original-based game payloads from fresh exact emitted ranges."""

from __future__ import annotations

import hashlib
import json
import os
from contextlib import ExitStack, closing
from pathlib import Path

from harness.build.arguments import inspect_arguments
from harness.build.compilation import configured_command
from harness.build.dependencies import IncludeSnapshot
from harness.build.operations import build, cmake_target_for_directory
from harness.common.directory import (
    close_descriptors,
    open_parent_chain,
    verify_parent_chain,
)
from harness.common.files import atomic_write
from harness.common.inputs import InputBatch, relative
from harness.common.lease import acquire_writer, require_writer
from harness.common.process import run_bounded
from harness.domain.claims import manifest_source_paths
from harness.domain.functions import parse_function_records
from harness.domain.manifests import _parse_manifest
from harness.domain.psx import is_psx_exe, payload_for, validate_psx_header
from harness.domain.registry import lookup_target_manifest
from harness.emi.catalog_verify import resolve_shipped_entry
from harness.io import repo_layout, unique_object
from harness.match.images import capture_source_images
from harness.match.members import capture_members

_LIMIT = 64 * 1024 * 1024


def _sha256(content: bytes) -> str:
    return hashlib.sha256(content).hexdigest()


def _bind(batch, pins, name, expected=None, *, required=True):
    name = relative(name)
    state, content = batch.read(batch.root / name, max_bytes=_LIMIT, single_link=True)
    if required and content is None:
        raise ValueError(f"payload input is absent: {name}")
    digest = None if state is None else state["sha256"]
    if name in pins and pins[name] != digest:
        raise ValueError(f"payload input changed: {name}")
    if expected is not None and digest != expected["sha256"]:
        raise ValueError(f"captured input differs from payload input: {name}")
    pins[name] = digest
    return content


def _bind_states(batch, pins, states):
    for name, state in states.items():
        content = _bind(batch, pins, name, state, required=state is not None)
        if state is None and content is not None:
            raise ValueError(f"absent captured input appeared: {name}")


def _verify_pins(batch, pins):
    for name, expected in pins.items():
        state, _ = batch.read(batch.root / name, max_bytes=_LIMIT, single_link=True)
        if (None if state is None else state["sha256"]) != expected:
            raise ValueError(f"payload input changed before completion: {name}")


def _create_output(root: Path, output: Path) -> None:
    name = output.relative_to(root).as_posix()
    descriptors, leaf = open_parent_chain(root, name, create=True)
    try:
        os.mkdir(leaf, dir_fd=descriptors[-1])
        verify_parent_chain(root, name, descriptors)
        os.fsync(descriptors[-1])
    finally:
        close_descriptors(descriptors)


def assemble_payload(manifest, normalized: bytes, original: bytes, ranges):
    """Validate placement and return a full shipped image and reconciled intervals."""
    for content, name in ((normalized, manifest.binary), (original, manifest.disc_id)):
        if is_psx_exe(content):
            validate_psx_header(content, manifest.load_address, binary_name=name)
    normalized_payload = payload_for(
        normalized, manifest.load_address, binary_name=manifest.binary
    )
    shipped_payload = payload_for(
        original, manifest.load_address, binary_name=manifest.disc_id
    )
    if manifest.kind == "executable" and not is_psx_exe(original):
        raise ValueError("shipped executable requires its original PS-X EXE header")
    if manifest.kind == "emi" and (is_psx_exe(normalized) or is_psx_exe(original)):
        raise ValueError("resident EMI code requires a raw payload")
    if (
        normalized[normalized_payload.binary_offset :]
        != original[shipped_payload.binary_offset :]
    ):
        raise ValueError("normalized binary differs from the shipped payload")
    if shipped_payload.payload_end > 0x100000000:
        raise ValueError("shipped payload exceeds the 32-bit address space")
    output = bytearray(original)
    compiled = []
    previous_end = shipped_payload.binary_offset
    for emitted in sorted(ranges, key=lambda item: item.original_file_start):
        first = emitted.original_file_start
        content = emitted.content
        if (
            type(content) is not bytes
            or not content
            or type(first) is not int
            or type(emitted.address) is not int
            or first
            != normalized_payload.binary_offset
            + emitted.address
            - manifest.load_address
            or first < normalized_payload.binary_offset
            or first + len(content) > len(normalized)
            or not emitted.owners
            or any(
                not owner.startswith(manifest.id.value + "@")
                for owner in emitted.owners
            )
            or (emitted.symbol is None) == (emitted.section is None)
        ):
            raise ValueError("compiled range has invalid placement or ownership")
        start = first - normalized_payload.binary_offset + shipped_payload.binary_offset
        end = start + len(content)
        if start < previous_end:
            raise ValueError("compiled ranges overlap")
        if (
            normalized[first : first + len(content)] != content
            or original[start:end] != content
        ):
            raise ValueError("emitted range differs from the complete original slice")
        output[start:end] = content
        compiled.append(
            {
                "start": start,
                "end": end,
                "address": emitted.address,
                "normalized_start": first,
                "source": emitted.source,
                "owners": list(emitted.owners),
                "symbol": emitted.symbol,
                "section": emitted.section,
                "sha256": _sha256(content),
            }
        )
        previous_end = end
    if not compiled:
        raise ValueError(
            "target has no nonempty freshly verified compiled contribution"
        )
    retained = []
    cursor = 0
    for interval in compiled:
        if cursor < interval["start"]:
            retained.append({"start": cursor, "end": interval["start"]})
        cursor = interval["end"]
    if cursor < len(original):
        retained.append({"start": cursor, "end": len(original)})
    if bytes(output) != original:
        raise ValueError("assembled payload differs from the complete original file")
    return bytes(output), compiled, retained


def _run_emi(root, tool, arguments, verify_tool):
    verify_tool()
    result = run_bounded(root, [tool, *arguments], output_limit=4 * 1024 * 1024)
    verify_tool()
    if result["failure"] or result["exit_code"]:
        raise RuntimeError(
            f"EMI payload operation failed: {result['failure'] or result['stderr']}"
        )
    return result["stdout"]


def _repack(
    root, output, entry, original_archive, payload_path, payload, batch, pins, stack
):
    tool = repo_layout(root).emi_ex_bin

    def read_tool():
        state, _ = batch.read(
            tool, max_bytes=_LIMIT, include_metadata=True, single_link=False
        )
        if state is None:
            raise ValueError(f"EMI tool is absent: {tool}")
        return state["sha256"], tuple(sorted(state["metadata"].items()))

    tool_state = read_tool()

    def verify_tool():
        if read_tool() != tool_state:
            raise ValueError("EMI tool changed during payload delivery")

    def verify_tool_on_success(error_type, error, traceback):
        if error_type is None:
            verify_tool()

    stack.push(verify_tool_on_success)
    manifest = json.loads(
        _run_emi(
            root,
            tool,
            ["extract", "--dry-run", "--print-manifest", entry["archive_path"]],
            verify_tool,
        ),
        object_pairs_hook=unique_object,
    )
    selected = [item for item in manifest["entries"] if item["index"] == entry["index"]]
    if len(selected) != 1 or any(
        selected[0][key] != entry["entry"][key]
        for key in ("index", "size", "ram_ptr", "type", "first4")
    ):
        raise ValueError("shipped EMI table differs from the extracted entry identity")
    archive_output = output / "archive.EMI"
    extracted = output / "entry.bin"
    _run_emi(
        root,
        tool,
        [
            "replace-entry",
            entry["archive_path"],
            str(entry["index"]),
            str(payload_path),
            "-o",
            str(archive_output),
        ],
        verify_tool,
    )
    archive_bytes = _bind(batch, pins, archive_output.relative_to(root).as_posix())
    if archive_bytes != original_archive:
        raise ValueError(
            "rebuilt EMI archive differs from the complete original archive"
        )
    _run_emi(
        root,
        tool,
        ["extract", str(archive_output), str(entry["index"]), "-o", str(extracted)],
        verify_tool,
    )
    if _bind(batch, pins, extracted.relative_to(root).as_posix()) != payload:
        raise ValueError("rebuilt EMI entry differs from the complete payload")
    return {
        "original": entry["archive_path"],
        "output": archive_output.relative_to(root).as_posix(),
        "index": entry["index"],
        "tool": tool.relative_to(root).as_posix(),
        "tool_sha256": tool_state[0],
        "tool_state": {"sha256": tool_state[0], "metadata": dict(tool_state[1])},
        "sha256": _sha256(archive_bytes),
        "whole_archive_equal": True,
        "entry_equal": True,
    }


def build_payload(root: Path, target: str, output: Path) -> dict:
    """Build explicit claims and publish a separately verified hybrid payload."""
    root = root.resolve(strict=True)
    output = root / output if not output.is_absolute() else output
    if (
        not output.is_relative_to(root / "out")
        or output == root / "out"
        or output.resolve() != output
    ):
        raise ValueError(
            "payload output must be a fresh canonical directory under out/"
        )
    with acquire_writer(root):
        _create_output(root, output)
        with ExitStack() as stack:
            batch = stack.enter_context(InputBatch(root))
            pins = {}
            manifest = lookup_target_manifest(root, target)
            if manifest is None:
                raise ValueError(f"unknown payload target: {target}")
            name = f"config/targets/{manifest.id.value}/target.toml"
            if _parse_manifest(root / name, _bind(batch, pins, name)) != manifest:
                raise ValueError("payload target manifest changed during resolution")
            normalized = _bind(batch, pins, manifest.binary)
            for name in (
                manifest.splat,
                "CMakeLists.txt",
                "config/compiler/object-flags.cmake",
                "config/compiler/variants.json",
                *manifest.headers,
            ):
                _bind(batch, pins, name)
            entry = None
            original_archive = None
            if manifest.kind == "emi":
                entry = resolve_shipped_entry(root, manifest.disc_id)
                _bind(
                    batch,
                    pins,
                    Path(entry["manifest_path"]).relative_to(root).as_posix(),
                )
                original_archive = _bind(
                    batch,
                    pins,
                    Path(entry["archive_path"]).relative_to(root).as_posix(),
                )
                original_name = Path(entry["payload_path"]).relative_to(root).as_posix()
                original = _bind(batch, pins, original_name)
                if (
                    resolve_shipped_entry(root, manifest.disc_id) != entry
                    or entry["load_address"] != manifest.load_address
                    or entry["size"] != len(original)
                    or entry["sha256"] != _sha256(original)
                    or entry["payload_kind"] != "ram"
                ):
                    raise ValueError(
                        "resident payload differs from the shipped EMI identity"
                    )
            else:
                original_name = "out/extracted/" + relative(manifest.disc_id)
                original = _bind(batch, pins, original_name)
            candidates = []
            for source in manifest_source_paths(root, manifest):
                name = source.relative_to(root).as_posix()
                content = _bind(batch, pins, name)
                if source.suffix != ".c":
                    continue
                arguments, _ = configured_command(
                    root, source, output / "unused.o", allow_grouped=True
                )
                operands = inspect_arguments(arguments[1:], cwd=root)
                includes = stack.enter_context(
                    closing(
                        IncludeSnapshot(
                            root,
                            source,
                            forced=tuple(item.path for item in operands.forced),
                        )
                    )
                )
                _bind_states(batch, pins, includes.describe_inputs())

                def verify_includes(error_type, error, traceback, snapshot=includes):
                    if error_type is None:
                        snapshot.validate()

                stack.push(verify_includes)
                if name == manifest.psyq_source:
                    continue
                records = parse_function_records(content.decode("utf-8"))
                if not records:
                    if name in manifest.sources:
                        raise ValueError(
                            f"authored payload source has no attached function metadata: {name}"
                        )
                    continue
                members = capture_members(root, name)
                if members["target"] != manifest.id.value:
                    raise ValueError("payload member belongs to a different target")
                _bind_states(batch, pins, members["inputs"])
                candidates.append(name)
            result = build(root, cmake_target_for_directory(manifest.source_dir))
            atomic_write(
                root,
                (output / "build.log").relative_to(root).as_posix(),
                (result.stdout + result.stderr).encode(),
                exclusive=True,
            )
            if result.returncode:
                raise RuntimeError(f"target build failed; see {output / 'build.log'}")
            ranges = []
            reports = []
            for source in candidates:
                comparison = capture_source_images(root, source, output)
                for key in ("native_inputs", "include_inputs"):
                    _bind_states(batch, pins, comparison.report[key])
                reports.append(
                    {
                        "source": source,
                        "comparison": comparison.report,
                        "compiled_range_count": len(comparison.ranges),
                    }
                )
                ranges.extend(comparison.ranges)

            # Bind capture digests before installing terminal checks over this batch.
            def verify_inputs(error_type, error, traceback):
                if error_type is None:
                    _verify_pins(batch, pins)

            stack.push(verify_inputs)
            payload, compiled, retained = assemble_payload(
                manifest, normalized, original, ranges
            )
            payload_path = output / "payload.bin"
            atomic_write(
                root, payload_path.relative_to(root).as_posix(), payload, exclusive=True
            )
            inputs = dict(pins)
            _bind(
                batch,
                pins,
                payload_path.relative_to(root).as_posix(),
                {"sha256": _sha256(payload)},
            )
            archive = (
                None
                if entry is None
                else _repack(
                    root,
                    output,
                    entry,
                    original_archive,
                    payload_path,
                    payload,
                    batch,
                    pins,
                    stack,
                )
            )
            require_writer(root)
            report = {
                "schema": "harness.build-payload/v1",
                "target": manifest.id.value,
                "kind": "hybrid",
                "original": original_name,
                "output": payload_path.relative_to(root).as_posix(),
                "size": len(payload),
                "sha256": _sha256(payload),
                "whole_payload_equal": True,
                "compiled": compiled,
                "retained": retained,
                "compiled_bytes": sum(item["end"] - item["start"] for item in compiled),
                "retained_bytes": sum(item["end"] - item["start"] for item in retained),
                "source_comparisons": reports,
                "inputs": inputs,
                "archive": archive,
            }
    # Only a completed input batch and released, verified lease permit success.
    atomic_write(
        root,
        (output / "report.json").relative_to(root).as_posix(),
        (json.dumps(report, indent=2, sort_keys=True) + "\n").encode(),
        exclusive=True,
    )
    return report
