"""Capture-bound projections for complete static predicate context."""

from __future__ import annotations

from harness.domain.ids import parse_function_id
from harness.naming.debt import address_of


def predicate_context(operation: dict, target: str, name: str) -> dict:
    """Bind one independently captured predicate body to the selected function."""
    from harness.naming.callereffects import decode_predicate_body

    selector = parse_function_id(f"{target}@{address_of(name):08X}")
    payload = operation.get("payload")
    if (
        operation.get("operation") != "function_predicate_context"
        or operation.get("role") != "function_predicate_context"
        or operation.get("supplemental") is not True
        or not isinstance(payload, dict)
        or set(payload) != {"selected", "consumer", "binding", "bytes", "gap"}
        or parse_function_id(operation["selector"]) != selector
        or parse_function_id(payload["selected"]) != selector
        or parse_function_id(payload["consumer"]) != selector
    ):
        raise ValueError("predicate context identity or role mismatch")
    if payload["gap"] is not None:
        raise ValueError(str(payload["gap"]))
    binding = payload["binding"]
    if not isinstance(binding, dict) or set(binding) != {
        "start",
        "end",
        "kind",
        "image_sha256",
        "splat_sha256",
        "replay_sha256",
    }:
        raise ValueError("predicate context binding is invalid")
    code = bytes.fromhex(payload["bytes"])
    if binding["start"] != selector.address or binding["end"] - binding["start"] != len(
        code
    ):
        raise ValueError("predicate context does not cover the complete selected range")
    return {
        "predicate": str(selector),
        "binding": binding,
        "context": decode_predicate_body(code, selector.address),
    }
