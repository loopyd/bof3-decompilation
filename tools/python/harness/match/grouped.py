"""Compile one preserved group and compare every complete native function range."""

from __future__ import annotations

import hashlib
import json
import tempfile
from contextlib import ExitStack, closing, nullcontext
from pathlib import Path

from harness.build.dependencies import IncludeSnapshot
from harness.build.dispatch import close_dispatch, prepare_dispatch, validate_dispatch
from harness.build.driver import protect_staging
from harness.build.preservation import (
    hash_preservation,
    read_preservation_document,
    validate_preservation_size,
)
from harness.build.receipts import produce_object, verify_production
from harness.build.routing import select_preservation, verify_selection
from harness.match.members import capture_members
from harness.common.files import read_file
from harness.common.files import atomic_write
from harness.common.inputs import InputBatch, relative
from harness.common.lease import ACTIVE_WRITER, acquire_writer, require_writer
from harness.common.observation import PathWatch
from harness.common.paths import require_absent
from harness.domain.functions import parse_function_records
from harness.domain.policy import validate_matching_text
from harness.io import RepoLayout, repo_layout
from harness.match._asm_diff_payload import (
    AsmDiffRequest,
    build_result_payload,
    render_diff,
)
from harness.match._asm_disasm import extract_instructions, render_normalized
from harness.match._asm_resolve import format_hex
from harness.match.execution import NativeExecution
from harness.match.extraction import read_function_image, read_linked_image
from harness.match.placement import extract_grouped_function, place_grouped_object

SCHEMA = "bof3.combiner-comparison/v7"
_LIMIT = 64 * 1024 * 1024
_OWNERS = (
    "tools/python/harness/match/grouped.py",
    "tools/python/harness/build/dependencies.py",
    "tools/python/harness/match/members.py",
    "tools/python/harness/combiner/cli.py",
    "tools/python/harness/build/receipts.py",
    "tools/python/harness/match/placement.py",
    "tools/python/harness/match/extraction.py",
    "tools/python/harness/match/execution.py",
    "tools/python/harness/match/flow.py",
    "tools/python/harness/common/lease.py",
    "tools/python/harness/common/process.py",
    "tools/python/harness/common/observation.py",
    "tools/python/harness/domain/policy.py",
    "tools/python/harness/domain/includes.py",
    "tools/python/harness/domain/symbols.py",
    "tools/python/harness/match/asm_diff.py",
    "tools/python/harness/match/bundle.py",
    "tools/python/harness/match/_asm_diff_payload.py",
    "tools/python/harness/match/_asm_disasm.py",
    "tools/python/harness/match/_asm_resolve.py",
    "tools/python/harness/commands/lift.py",
)


def _capture_states(root: Path, paths: set[Path]) -> dict:
    result = {}
    with InputBatch(root) as batch:
        for path in sorted(paths):
            state, _ = batch.read(path, max_bytes=_LIMIT, include_metadata=True)
            if state is None:
                raise ValueError(f"comparison input or output is absent: {path}")
            if state["metadata"]["st_nlink"] != 1:
                raise ValueError(f"comparison input or output is hardlinked: {path}")
            result[path.relative_to(root).as_posix()] = state
    return result


def resolve_comparison_outputs(root: Path, name: str) -> tuple[Path, Path, set[Path]]:
    if (
        relative(name) != name
        or not name.startswith("out/")
        or Path(name).suffix != ".o"
    ):
        raise ValueError("comparison requires a canonical out/ object path")
    output = root / name
    if output.resolve() != output or not output.parent.is_dir():
        raise ValueError("comparison output requires an existing canonical parent")
    linked = output.with_name(output.name + ".elf")
    paths = {
        output,
        output.with_name(output.name + ".s"),
        output.with_name(output.name + ".producer.json"),
        linked,
        linked.with_suffix(linked.suffix + ".ld"),
    }
    for path in paths:
        require_absent(root, path.relative_to(root).as_posix())
    return output, linked, paths


def _compare_function(
    layout, execution, linked: Path, member: dict, *, data_sections=None
) -> dict:
    image = read_function_image(
        linked, function_name=member["symbol"], address=member["address"]
    )
    original = member["original"]
    current = image.content
    if len(current) > execution.output_limit:
        raise ValueError("compiled function exceeds the comparison output bound")
    flow_verified = False
    if len(current) == len(original):
        current = extract_grouped_function(
            linked,
            member["symbol"],
            address=member["address"],
            size=len(original),
            layout=layout,
            execution=execution,
            data_sections=data_sections,
        )
        if current != image.content:
            raise ValueError("linked function changed during extraction")
        flow_verified = True
    exact = current == original
    mismatch = None
    if not exact:
        mismatch = next(
            (
                offset
                for offset, (expected, actual) in enumerate(zip(original, current))
                if expected != actual
            ),
            min(len(original), len(current)),
        )
    return {
        "selector": member["selector"],
        "symbol": member["symbol"],
        "address": member["address"],
        "original_file_start": member["file_start"],
        "original_size": len(original),
        "current_size": len(current),
        "size_delta": len(current) - len(original),
        "original_sha256": hashlib.sha256(original).hexdigest(),
        "current_sha256": hashlib.sha256(current).hexdigest(),
        "byte_match": exact,
        "flow_verified": flow_verified,
        "first_mismatch_offset": mismatch,
    }


def compare_members(
    root: Path,
    source: str,
    output: str,
    *,
    output_limit: int = 2 * 1024 * 1024,
) -> dict:
    """Require a writer lease and fresh routed production; never accept source edits."""
    root = root.resolve(strict=True)
    require_writer(root)
    if type(output_limit) is not int or not 1 <= output_limit <= _LIMIT:
        raise ValueError("comparison output limit must be between 1 and 64 MiB")
    return _compare_members(root, source, output, output_limit)


def _compare_data(
    linked: Path, sections: dict, execution: NativeExecution
) -> list[dict]:
    if not sections:
        return []
    image = read_linked_image(
        read_file(
            execution.root,
            linked.relative_to(execution.root).as_posix(),
            max_bytes=_LIMIT,
        ),
        limit=_LIMIT,
    )
    comparisons = []
    for name, expected in sorted(sections.items()):
        candidates = [section for section in image.sections if section.name == name]
        if len(candidates) != 1:
            raise ValueError(
                "linked image omits or duplicates reviewed initialized data"
            )
        section = candidates[0]
        if (
            section.kind != 1
            or not section.flags & 2
            or section.flags & (4 | 0xC00)
            or section.address != expected["address"]
            or section.content is None
            or section.size > execution.output_limit
            or any(row["target_section"] == section.index for row in image.relocations)
        ):
            raise ValueError(
                "linked initialized data has unsupported storage or relocation"
            )
        original, current = expected["original"], section.content
        mismatch = (
            None
            if original == current
            else next(
                (
                    offset
                    for offset, pair in enumerate(zip(original, current))
                    if pair[0] != pair[1]
                ),
                min(len(original), len(current)),
            )
        )
        comparisons.append(
            {
                "section": name,
                "owners": expected["owners"],
                "address": section.address,
                "original_file_start": expected["file_start"],
                "original_size": len(original),
                "current_size": section.size,
                "size_delta": section.size - len(original),
                "original_sha256": hashlib.sha256(original).hexdigest(),
                "current_sha256": hashlib.sha256(current).hexdigest(),
                "byte_match": original == current,
                "first_mismatch_offset": mismatch,
            }
        )
    return comparisons


def compare_selected_member(request: AsmDiffRequest, *, layout: RepoLayout) -> dict:
    """Compare the complete preserved group before reporting its selected member."""
    root = layout.root.resolve(strict=True)
    source = request.source_path.expanduser().resolve(strict=True)
    if type(request.address) is not int:
        raise ValueError(
            "grouped native comparison requires an explicit member address"
        )
    output_root = (request.output_root or layout.out_dir / "matching").absolute()
    if (
        not output_root.is_relative_to(root / "out")
        or output_root.resolve() != output_root
    ):
        raise ValueError("grouped comparison diagnostics require a canonical out/ path")
    lease = acquire_writer(root) if ACTIVE_WRITER.get() is None else nullcontext()
    with lease:
        require_writer(root)
        output_root.mkdir(parents=True, exist_ok=True)
        output_dir = Path(tempfile.mkdtemp(prefix="grouped-", dir=output_root))
        return _compare_members(
            root,
            source.relative_to(root).as_posix(),
            (output_dir / "group.o").relative_to(root).as_posix(),
            2 * 1024 * 1024,
            request=request,
        )


def _select_request(root: Path, request: AsmDiffRequest, members: dict) -> dict:
    selected = [
        member
        for member in members["functions"]
        if member["address"] == request.address
    ]
    if len(selected) != 1:
        raise ValueError("selected address is absent from the complete preserved group")
    member = selected[0]
    if (
        request.binary_path is not None
        and request.binary_path.expanduser().resolve() != root / members["binary"]
    ):
        raise ValueError(
            "grouped comparison cannot replace the preserved original image"
        )
    if (
        request.load_address is not None
        and request.load_address != members["load_address"]
    ):
        raise ValueError("grouped comparison cannot replace the preserved load address")
    if request.size is not None and (
        type(request.size) is not int or request.size != len(member["original"])
    ):
        raise ValueError(
            "grouped comparison requires the complete reviewed member range"
        )
    if request.canonical_bindings is not None and (
        any(type(value) is not int for value in request.canonical_bindings.values())
        or dict(request.canonical_bindings) != members["bindings"]
    ):
        raise ValueError(
            "grouped comparison bindings differ from the preserved target maps"
        )
    if request.symbols_c_path is not None:
        raise ValueError(
            "grouped comparison requires target-map bindings without a C override"
        )
    if (
        request.section_placements is not None
        and tuple(request.section_placements)
        != members["section_placements"][member["address"]]
    ):
        raise ValueError("grouped data placement differs from the preserved manifest")
    if type(request.diagnostics) is not bool:
        raise ValueError("grouped comparison diagnostics must be boolean")
    return member


def _render_selected(root, request, member, result, output, linked, execution):
    output_dir = output.parent
    validate_preservation_size(result)

    def write(name, content):
        atomic_write(
            root,
            (output_dir / name).relative_to(root).as_posix(),
            content,
            exclusive=True,
        )

    write(
        "fullgroup.json", (json.dumps(result, indent=2, sort_keys=True) + "\n").encode()
    )
    comparison = next(
        item for item in result["members"] if item["address"] == member["address"]
    )
    if comparison["byte_match"] and not result["all_native_bytes_match"]:
        failed = ", ".join(
            item["selector"]
            for item in result["members"]
            if not item["byte_match"] or not item["flow_verified"]
        )
        failed_data = ", ".join(
            item["section"]
            for item in result["data_sections"]
            if not item["byte_match"]
        )
        reason = "; ".join(
            label
            for label in (
                f"differing members: {failed}" if failed else "",
                f"differing initialized data: {failed_data}" if failed_data else "",
            )
            if label
        )
        raise ValueError(
            f"grouped source has {reason}; see {output_dir / 'fullgroup.json'}"
        )
    exact = result["all_native_bytes_match"]
    payload = {
        "schema": "harness.byte-match-one/v1",
        "status": "exact_match" if exact else "different",
        "exact_match": exact,
        "byte_match": exact,
        "source": str(request.source_path.expanduser().resolve()),
        "function": member["symbol"],
        "address": format_hex(member["address"]),
        "original_size": comparison["original_size"],
        "current_size": comparison["current_size"],
        "size_delta": comparison["size_delta"],
        "original_binary": str(root / result["original_binary"]),
        "current_object": str(output),
        "outputs": {},
    }
    if not request.diagnostics:
        return payload
    write("original.bin", member["original"])
    objdump = repo_layout(root).psyqo_toolchain_root / "bin/mipsel-none-elf-objdump"
    original_dump = execution.run_text(
        [
            objdump,
            "-D",
            "-b",
            "binary",
            "-m",
            "mips:3000",
            "-EL",
            f"--adjust-vma={format_hex(member['address'])}",
            output_dir / "original.bin",
        ]
    )
    current_dump = execution.run_text(
        [
            objdump,
            "-d",
            f"--start-address=0x{member['address']:X}",
            f"--stop-address=0x{member['address'] + comparison['current_size']:X}",
            linked,
        ]
    )
    original_lines, current_lines = (
        extract_instructions(original_dump),
        extract_instructions(current_dump),
    )
    write("original.s", render_normalized(original_lines).encode())
    write("current.s", render_normalized(current_lines).encode())
    write("linked.s", current_dump.encode())
    write("diff.patch", render_diff(original_lines, current_lines).encode())
    write("compiler.s", output.with_name(output.name + ".s").read_bytes())
    write(
        "build.log",
        f"Fresh grouped producer receipt: {result['producer']['receipt']}\nComplete group comparison: {output_dir / 'fullgroup.json'}\n".encode(),
    )
    payload = build_result_payload(
        source_path=Path(payload["source"]),
        function_name=member["symbol"],
        address=member["address"],
        original_size=comparison["original_size"],
        current_size=comparison["current_size"],
        byte_match=exact,
        binary_path=Path(payload["original_binary"]),
        object_path=output,
        output_dir=output_dir,
        original_lines=original_lines,
        current_lines=current_lines,
        linked_path=linked,
    )
    write("summary.json", (json.dumps(payload, indent=2) + "\n").encode())
    return payload


def _compare_members(root, source, output_name, output_limit, *, request=None):
    if (
        relative(source) != source
        or not source.startswith("src/bof3/")
        or not source.endswith(".c")
    ):
        raise ValueError("comparison requires one canonical src/bof3/ C source")
    source_path = root / source
    records = parse_function_records(
        read_file(root, source, max_bytes=4 * 1024 * 1024).decode("utf-8")
    )
    if len(records) < 2:
        raise ValueError("comparison requires a complete grouped translation unit")
    output, linked, artifacts = resolve_comparison_outputs(root, output_name)
    layout = repo_layout(root)
    execution = NativeExecution(root, output_limit)
    tools = layout.psyqo_toolchain_root / "bin"
    with ExitStack() as stack:
        selection = stack.enter_context(
            closing(select_preservation(root, [source_path], [source_path]))
        )
        if selection.record is None:
            raise ValueError(
                "comparison requires externally pinned preservation routing"
            )
        record = read_preservation_document(selection.record)
        preserved = verify_selection(root, selection)
        if preserved["destination"] != source:
            raise ValueError("comparison source differs from its preserved destination")
        arguments = [
            *preserved["profile"]["arguments"][1:],
            "-c",
            str(source_path),
            "-o",
            str(output),
        ]
        dispatch = prepare_dispatch(root, arguments)
        stack.callback(close_dispatch, dispatch)
        if (
            dispatch.record_path != selection.record
            or dispatch.record_fingerprint != selection.fingerprint
        ):
            raise ValueError(
                "comparison compiler selected different preservation history"
            )
        includes = stack.enter_context(
            closing(
                IncludeSnapshot(
                    root,
                    source_path,
                    forced=tuple(
                        Path(item["path"]) for item in dispatch.state["forced_inputs"]
                    ),
                )
            )
        )
        extra_paths = (
            set(includes.content)
            | {root / name for name in _OWNERS}
            | {tools / f"mipsel-none-elf-{name}" for name in ("ld", "objdump", "nm")}
        )
        extra_paths.update(
            root / name
            for name, state in preserved["inputs"].items()
            if state is not None
        )
        protected = (
            {root / name for name in preserved["inputs"]}
            | extra_paths
            | includes.paths
            | selection.reserved
            | {selection.record}
        )
        if selection.routes is not None:
            protected.add(selection.routes)
        if artifacts & protected:
            raise ValueError("comparison artifact collides with a retained input")
        protected_inputs = tuple(sorted(protected))
        dispatch.programs.protect_outputs(list(artifacts), inputs=protected_inputs)
        protect_staging(dispatch, inputs=protected_inputs)
        native_watch = stack.enter_context(closing(PathWatch(extra_paths)))
        native_inputs = _capture_states(root, extra_paths)
        includes.validate()
        for path, content in includes.content.items():
            if not path.is_relative_to(root / "toolchains"):
                validate_matching_text(content.decode("utf-8"))
        members = capture_members(root, record, preserved)
        selected = None if request is None else _select_request(root, request, members)

        def guard():
            require_writer(root)
            selection.validate()
            dispatch.watch.validate()
            dispatch.programs.validate(dispatch.invocation)
            native_watch.validate()
            includes.check()

        guard()
        validate_dispatch(dispatch)
        produced = produce_object(root, arguments)
        guard()
        if produced["invocation_fingerprint"] != dispatch.fingerprint:
            raise ValueError("fresh producer invocation differs from comparison inputs")
        producer_paths = artifacts - {linked, linked.with_suffix(linked.suffix + ".ld")}
        producer_watch = stack.enter_context(closing(PathWatch(producer_paths)))
        production = verify_production(root, arguments, produced["sha256"])
        if (
            production["receipt"] != produced["receipt"]
            or production["invocation_fingerprint"] != dispatch.fingerprint
            or production["execution_fingerprint"] != produced["execution_fingerprint"]
        ):
            raise ValueError(
                "fresh producer receipt differs from comparison provenance"
            )
        producer_states = _capture_states(root, producer_paths)
        producer_watch.validate()
        guard()
        functions = {
            member["symbol"]: member["address"] for member in members["functions"]
        }
        if len(functions) != len(members["functions"]):
            raise ValueError("comparison member symbols are duplicated")
        data_placements = {
            name: section["address"]
            for name, section in members["data_sections"].items()
        }
        placed = place_grouped_object(
            output,
            linked,
            functions,
            layout=layout,
            execution=execution,
            bindings=members["bindings"],
            data_sections=data_placements,
        )
        if placed["path"] != str(linked) or placed["script"] != str(
            linked.with_suffix(linked.suffix + ".ld")
        ):
            raise ValueError("native placement published a different linked output")
        guard()
        producer_watch.validate()
        linked_paths = {linked, linked.with_suffix(linked.suffix + ".ld")}
        linked_watch = stack.enter_context(closing(PathWatch(linked_paths)))
        linked_states = _capture_states(root, linked_paths)
        if (
            linked_states[linked.relative_to(root).as_posix()]["sha256"]
            != placed["sha256"]
            or linked_states[
                linked.with_suffix(linked.suffix + ".ld").relative_to(root).as_posix()
            ]["sha256"]
            != placed["script_sha256"]
        ):
            raise ValueError("linked outputs differ from freshly placed images")
        comparisons = []
        for member in members["functions"]:
            guard()
            producer_watch.validate()
            linked_watch.validate()
            comparisons.append(
                _compare_function(
                    layout,
                    execution,
                    linked,
                    member,
                    data_sections=data_placements,
                )
            )
        data_comparisons = _compare_data(linked, members["data_sections"], execution)
        guard()
        producer_watch.validate()
        linked_watch.validate()
        validate_dispatch(dispatch)
        includes.validate()
        if (
            _capture_states(root, extra_paths) != native_inputs
            or _capture_states(root, producer_paths) != producer_states
            or _capture_states(root, linked_paths) != linked_states
        ):
            raise ValueError("comparison native inputs or outputs changed")
        functions_exact = all(
            member["byte_match"] and member["flow_verified"] for member in comparisons
        )
        data_exact = all(section["byte_match"] for section in data_comparisons)
        exact = functions_exact and data_exact
        result = {
            "schema": SCHEMA,
            "scope": "all-selected-function-and-initialized-data-bytes",
            "target": preserved["target"],
            "source": source,
            "original_binary": members["binary"],
            "load_address": members["load_address"],
            "preservation_fingerprint": selection.fingerprint,
            "producer": production,
            "placement": placed,
            "fresh_compilation": True,
            "configured_preserved": True,
            "member_count": len(comparisons),
            "members": comparisons,
            "status": "exact" if exact else "different",
            "all_function_bytes_match": functions_exact,
            "data_sections": data_comparisons,
            "all_data_bytes_match": data_exact,
            "all_native_bytes_match": exact,
            "native_inputs": native_inputs,
            "include_inputs": includes.describe_inputs(),
            "source_image": includes.image.describe(),
            "outputs": {**producer_states, **linked_states},
            "full_input_closure_verified": False,
            "consumer_coverage_verified": False,
            "reusable": False,
            "accepted": False,
            "write_authorized": False,
        }
        result["fingerprint"] = hash_preservation(result)
        try:
            return (
                result
                if request is None
                else _render_selected(
                    root, request, selected, result, output, linked, execution
                )
            )
        finally:
            guard()
            producer_watch.validate()
            linked_watch.validate()
            includes.validate()
            validate_dispatch(dispatch)
            if (
                _capture_states(root, extra_paths) != native_inputs
                or _capture_states(root, producer_paths) != producer_states
                or _capture_states(root, linked_paths) != linked_states
            ):
                raise ValueError(
                    "comparison native inputs or outputs changed during rendering"
                )
