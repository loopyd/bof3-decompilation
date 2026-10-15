"""Capture and replay native measurements for retained partial functions."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from harness.common.checks import validate_partial_evidence
from harness.common.digests import digest
from harness.domain.registry import resolve_function
from harness.domain.tags import count_function_metadata, parse_progress_tags
from harness.io import repo_layout
from harness.match._asm_resolve import object_path_for_source
from harness.match.extraction import read_function_image
from harness.naming.instructions import resolve_instructions
from harness.naming.native import NativeByteOps
from harness.naming.plan import semantic_operation_plan


def _validate_output(root: Path, operation: dict, result: dict):
    """Validate exact selected identity and canonical binary/object paths."""
    if result.get("killed") is not False or result.get("semantic_success") is not True:
        raise ValueError("partial native command did not finish successfully")
    resolved = resolve_function(root, operation["target"])
    source, symbol = resolved.source, resolved.compiled_symbol
    if source is None or symbol is None:
        raise ValueError("partial function requires a claimed source and symbol")
    validate_partial_evidence(
        root,
        tool=operation["kind"],
        target=resolved.target.id.value,
        selector=operation["target"],
        function=symbol,
        source=source.relative_to(root).as_posix(),
        exit_code=result.get("exit"),
        output=result["raw"],
    )
    payload = json.loads(result["raw"])
    object_path = object_path_for_source(repo_layout(root), source).resolve()
    binary = (root / payload["original_binary"]).resolve()
    if (
        binary != resolved.target.binary_path.resolve()
        or (root / payload["current_object"]).resolve() != object_path
    ):
        raise ValueError("partial native result names a different binary or object")
    return resolved, payload, object_path


def capture_partial(root: Path, operation: dict, result: dict) -> dict:
    """Snapshot bytes immediately after one successful native mismatch command."""
    resolved, payload, object_path = _validate_output(root, operation, result)
    source, symbol = resolved.source, resolved.compiled_symbol
    source_bytes = source.read_bytes()
    text = source_bytes.decode("utf-8")
    if count_function_metadata(text) != 1:
        raise ValueError("partial baseline requires an ordinary single-function source")
    progress = parse_progress_tags(text)
    if progress is None or progress[0] != "partial":
        raise ValueError("partial baseline requires current partial progress metadata")
    binding, original = resolve_instructions(root, operation["target"])
    current = read_function_image(
        object_path.with_suffix(".linked.o"),
        function_name=symbol,
        address=resolved.id.address,
    ).content
    if (
        payload["original_size"] != len(original)
        or payload["current_size"] != len(current)
        or (current == original) is not payload["byte_match"]
    ):
        raise ValueError("partial native measurements disagree with function bytes")
    if operation["kind"] == "asm-diff":
        original_path = root / payload["outputs"]["original_bytes"]
        if original_path.read_bytes() != original:
            raise ValueError(
                "partial diagnostic original bytes differ from the payload"
            )
    snapshot = {
        "selector": operation["target"],
        "source": source.relative_to(root).as_posix(),
        "source_sha256": hashlib.sha256(source_bytes).hexdigest(),
        "binding": binding,
        "original_sha256": hashlib.sha256(original).hexdigest(),
        "current_sha256": hashlib.sha256(current).hexdigest(),
        "declared": dict(
            zip(("status", "match_percent", "residual"), progress, strict=True)
        ),
        "status": payload["status"],
        "byte_match": payload["byte_match"],
        "original_size": len(original),
        "current_size": len(current),
        "size_delta": payload["size_delta"],
    }
    if operation["kind"] == "asm-diff":
        snapshot["measurement"] = {
            "instruction_count": payload["instruction_count"],
            "first_mismatch": payload["first_mismatch"],
        }
    return snapshot


def partial_observation(row: dict, target: str, semantic: list) -> tuple[dict, ...]:
    """Require both separately executed commands to measure the same bytes."""
    if row.get("kind") != "function" or row.get("partial_used") is not True:
        return ()
    planned = [
        operation
        for operation in semantic_operation_plan(row, target)
        if operation["kind"] in {"asm-diff", "byte-match"}
    ]
    snapshots = []
    for operation in planned:
        matches = [
            item for item in semantic if item.get("command") == operation["command"]
        ]
        if len(matches) != 1:
            raise ValueError("partial native pair differs from the generated plan")
        item = matches[0]
        snapshot = item.get("partial_snapshot")
        if not isinstance(snapshot, dict):
            return ()
        if (
            item.get("selector") != operation["target"]
            or item.get("exit") != 1
            or item.get("killed") is not False
            or item.get("semantic_success") is not True
        ):
            raise ValueError("partial native pair has invalid execution status")
        snapshots.append(snapshot)
    if len(snapshots) != 2:
        raise ValueError("partial baseline requires the canonical native pair")
    asm, byte = snapshots
    if digest(
        {key: value for key, value in asm.items() if key != "measurement"}
    ) != digest(byte):
        raise ValueError("partial native pair measured different inputs or bytes")
    if "measurement" not in asm:
        raise ValueError("partial baseline lacks assembly measurements")
    return ({"class": "partial_baseline", "polarity": "positive", "value": asm},)


def partial_projection(observations: tuple[dict, ...]) -> dict:
    """Project the partial fact only onto its assembly measurement receipt."""
    return {
        "schema": "bof3.naming-evidence-facts/v1",
        "facts": ["partial_baseline"],
        "observations": list(observations),
        "open": [],
    }


def replay_partial(
    root: Path, row: dict, target: str, payload: dict, *, native_ops=None
) -> tuple[dict, ...]:
    """Rerun the code-owned pair before accepting retained positive measurements."""
    retained = partial_observation(row, target, payload["semantic"])
    if not retained:
        return ()
    operations = [
        operation
        for operation in semantic_operation_plan(row, target)
        if operation["kind"] in {"asm-diff", "byte-match"}
    ]
    runner = native_ops if native_ops is not None else NativeByteOps(root)
    current = []
    for operation in operations:
        saved = next(
            item
            for item in payload["semantic"]
            if item["command"] == operation["command"]
        )
        raw_path = root / saved["raw_file"]
        raw = raw_path.read_text()
        _validate_output(root, operation, {**saved, "raw": raw})
        commands = [
            item
            for item in payload["commands"]
            if item.get("command") == operation["command"]
        ]
        if (
            len(commands) != 1
            or commands[0].get("output") != raw
            or commands[0].get("status") != "passed"
            or commands[0].get("selector") != operation["target"]
        ):
            raise ValueError(
                "partial receipt does not retain the complete native output"
            )
        receipt = json.loads((root / commands[0]["receipt"]).read_text())
        if operation["kind"] == "asm-diff":
            expected = partial_projection(retained)
            if any(
                digest(receipt.get(key)) != digest(value)
                for key, value in expected.items()
            ):
                raise ValueError(
                    "partial typed receipt differs from measured observations"
                )
        elif any(key in receipt for key in ("facts", "observations", "open")):
            raise ValueError(
                "byte-match receipt cannot carry partial measurement facts"
            )
        result = runner.run(operation)
        record = {
            "command": operation["command"],
            "selector": operation["target"],
            "exit": result.exit,
            "killed": result.killed,
            "semantic_success": result.exit == 1 and not result.killed,
            "raw": result.raw,
        }
        record["partial_snapshot"] = capture_partial(root, operation, record)
        # Validate retained JSON against its snapshot, independently of the fresh run.
        saved_payload = json.loads(raw)
        snapshot = saved["partial_snapshot"]
        for key in (
            "status",
            "byte_match",
            "original_size",
            "current_size",
            "size_delta",
        ):
            if digest(saved_payload.get(key)) != digest(snapshot[key]):
                raise ValueError("partial retained output differs from its snapshot")
        if operation["kind"] == "asm-diff" and any(
            digest(saved_payload.get(key)) != digest(value)
            for key, value in snapshot["measurement"].items()
        ):
            raise ValueError("partial retained assembly measurements differ")
        current.append(record)
    observations = partial_observation(row, target, current)
    if digest(observations) != digest(retained):
        raise ValueError("partial native measurements are stale")
    return observations
