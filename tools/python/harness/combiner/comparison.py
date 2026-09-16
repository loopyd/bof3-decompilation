"""Compile one preserved group and compare every complete native function range."""

from __future__ import annotations

import hashlib
from contextlib import ExitStack, closing
from pathlib import Path

from harness.build.dependencies import IncludeSnapshot
from harness.build.dispatch import close_dispatch, prepare_dispatch, validate_dispatch
from harness.build.preservation import (
    hash_preservation,
    read_preservation_document,
    verify_preservation,
)
from harness.build.receipts import produce_object, verify_production
from harness.build.routing import select_preservation
from harness.combiner.members import capture_members
from harness.common.deadlines import check_deadline, resolve_deadline, use_deadline
from harness.common.files import read_file
from harness.common.inputs import InputBatch, relative
from harness.common.lease import require_writer
from harness.common.observation import PathWatch
from harness.common.paths import require_absent
from harness.domain.functions import parse_function_records
from harness.domain.policy import validate_matching_text
from harness.io import repo_layout
from harness.match.execution import NativeExecution
from harness.match.extraction import read_function_image
from harness.match.placement import extract_grouped_function, place_grouped_object

SCHEMA = "bof3.combiner-comparison/v6"
_LIMIT = 64 * 1024 * 1024
_OWNERS = (
    "tools/python/harness/combiner/comparison.py",
    "tools/python/harness/build/dependencies.py",
    "tools/python/harness/combiner/members.py",
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


def _compare_function(layout, execution, linked: Path, member: dict) -> dict:
    execution.check_deadline()
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
    execution.check_deadline()
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
    deadline: float | None = None,
    output_limit: int = 2 * 1024 * 1024,
) -> dict:
    """Require a writer lease and fresh routed production; never accept source edits."""
    root = root.resolve(strict=True)
    require_writer(root)
    cutoff = resolve_deadline(deadline)
    if cutoff is None:
        raise ValueError("grouped comparison requires an original work deadline")
    if type(output_limit) is not int or not 1 <= output_limit <= _LIMIT:
        raise ValueError("comparison output limit must be between 1 and 64 MiB")
    with use_deadline(cutoff):
        return _compare_members(root, source, output, cutoff, output_limit)


def _compare_members(root, source, output_name, cutoff, output_limit):
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
    execution = NativeExecution(root, cutoff, output_limit)
    tools = layout.psn00b_toolchain_root / "bin"
    with ExitStack() as stack:
        selection = stack.enter_context(
            closing(select_preservation(root, [source_path], [source_path]))
        )
        if selection.record is None:
            raise ValueError(
                "comparison requires externally pinned preservation routing"
            )
        record = read_preservation_document(selection.record)
        preserved = verify_preservation(
            root, record, expected_fingerprint=selection.fingerprint
        )
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
        dispatch.programs.protect_outputs(list(artifacts))
        native_watch = stack.enter_context(closing(PathWatch(extra_paths)))
        native_inputs = _capture_states(root, extra_paths)
        includes.validate()
        for path, content in includes.content.items():
            if not path.is_relative_to(root / "toolchains"):
                validate_matching_text(content.decode("utf-8"))
        members = capture_members(root, record, preserved)

        def guard():
            check_deadline()
            execution.check_deadline()
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
        placed = place_grouped_object(
            output,
            linked,
            functions,
            layout=layout,
            execution=execution,
            bindings=members["bindings"],
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
            comparisons.append(_compare_function(layout, execution, linked, member))
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
        exact = all(
            member["byte_match"] and member["flow_verified"] for member in comparisons
        )
        result = {
            "schema": SCHEMA,
            "scope": "all-selected-function-bytes",
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
            "all_function_bytes_match": exact,
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
        guard()
        producer_watch.validate()
        linked_watch.validate()
        return result
