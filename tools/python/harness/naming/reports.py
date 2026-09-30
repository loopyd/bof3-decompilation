"""Validate naming audits and immutable transaction receipt bindings."""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

from harness.common.commands import command_text, tool_id, trailing_arguments
from harness.domain.manifests import load_target_manifests
from harness.domain.receipts import command_records
from harness.domain.symbols import load_target_symbols
from harness.naming.checks import collect_function_checks
from harness.naming.context import (
    DIGEST_VERSION,
    READY_STATUS,
    SCHEMA_V3,
    TargetContext,
    inventory_expected,
    naming_manifest,
    pre_apply,
    reviewed_scope_digest,
    validate_row_v3,
)
from harness.naming.facts import range_contains
from harness.naming.data import validate_data_preservation, validate_data_shape

_SPACING = re.compile(r"\s+")


def _complete(rows: list[dict[str, Any]]) -> bool:
    return not any(
        row.get("rung_status") == "blocked" for row in rows if isinstance(row, dict)
    )


def validate(
    root: Path,
    target: str,
    report: dict[str, Any],
    ctx: TargetContext,
    *,
    transaction: str | None = None,
    post_apply: bool = False,
    expected: set[tuple[str, str]] | None = None,
) -> dict[str, Any]:
    rows = report.get("rows")
    if not isinstance(rows, list):
        raise ValueError("rows must be an array")
    if transaction is None:
        _full_report(root, target, report, rows, ctx, expected=expected)
        return {
            "schema": SCHEMA_V3,
            "target": target,
            "rows": len(rows),
            "complete": _complete(rows),
        }
    return _transaction_check(
        target, report, rows, ctx, transaction, post_apply=post_apply
    )


def _full_report(
    root: Path,
    target: str,
    report: dict[str, Any],
    rows: list[dict[str, Any]],
    ctx: TargetContext,
    *,
    expected: set[tuple[str, str]] | None = None,
) -> None:
    if report.get("schema") != SCHEMA_V3:
        raise ValueError(f"report schema must be {SCHEMA_V3}")
    if report.get("target") != ctx.target:
        raise ValueError("report target does not match selector")
    if expected is None:
        manifests = load_target_manifests(root)
        expected = inventory_expected(root, target, manifests)
    seen: set[tuple[str, str]] = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("rows must be objects")
        key = (str(row.get("kind")), str(row.get("name")))
        if key in seen:
            raise ValueError(f"duplicate row: {key[0]}:{key[1]}")
        seen.add(key)
        validate_row_v3(row, key[0], key[1], ctx)
    if seen != expected:
        missing = sorted(expected - seen)
        extra = sorted(seen - expected)
        raise ValueError(f"inventory mismatch: missing={missing} extra={extra}")
    if (report.get("complete") is True) != _complete(rows):
        raise ValueError(
            f"complete must be {str(_complete(rows)).lower()} from row states"
        )


def _transaction_check(
    target: str,
    report: dict[str, Any],
    rows: list[dict[str, Any]],
    ctx: TargetContext,
    selector: str,
    *,
    post_apply: bool = False,
) -> dict[str, Any]:
    try:
        kind, name = selector.split(":", 1)
    except ValueError as error:
        raise ValueError("--transaction must be KIND:NAME") from error
    selected = [
        row
        for row in rows
        if isinstance(row, dict) and (row.get("kind"), row.get("name")) == (kind, name)
    ]
    if len(selected) != 1:
        raise ValueError(f"row not found exactly once: {selector}")
    row = selected[0]
    if row.get("rung_status") != "proposed":
        raise ValueError(f"{selector} is not a proposed row")
    if post_apply:
        _post_apply_check(row, kind, name, ctx)
        return {
            "schema": SCHEMA_V3,
            "target": target,
            "transaction": f"{kind}:{name}",
            "applied": True,
            "rows": 1,
        }
    blocked = validate_row_v3(row, kind, name, ctx)
    for other in rows:
        if other is row or not isinstance(other, dict):
            continue
        if other.get("rung_status") != "proposed":
            continue
        other_scope = ctx.scope(str(other.get("name")))
        scope = ctx.scope(name)
        overlap = set(
            other_scope["binding_locations"] + other_scope["source_locations"]
        ) & set(scope["binding_locations"] + scope["source_locations"])
        if overlap:
            raise ValueError(
                f"{selector} overlaps proposed row "
                f"{other.get('kind')}:{other.get('name')} on {sorted(overlap)}"
            )
    result = {
        "schema": SCHEMA_V3,
        "target": target,
        "transaction": f"{kind}:{name}",
        "ready": not blocked
        and row.get("transaction_status") == "ready"
        and row.get("semantic_status") == "accepted",
        "rows": 1,
    }
    if result["ready"]:
        # Only an accepted, ready proposal receives an apply receipt.
        binding = pre_apply(ctx, kind, name, row)
        result["pre_apply"] = binding
        # Tool-owned versioned manifest: the same facts plus the collision
        # result and required checks, so the apply step is fully specified
        # without agent prose.  verify re-derives and compares it.
        result["manifest"] = naming_manifest(ctx, kind, name, row, binding)
    return result


def _post_apply_check(
    row: dict[str, Any], kind: str, old_name: str, ctx: TargetContext
) -> None:
    identity = row.get("identity")
    if not isinstance(identity, dict):
        raise ValueError(f"post-apply row requires identity for {kind}:{old_name}")
    new_name = row.get("new_name")
    if not isinstance(new_name, str) or identity.get("new") != new_name:
        raise ValueError(f"{old_name} post-apply new_name must equal identity.new")
    selector = identity.get("selector")
    if not isinstance(selector, str) or not selector.startswith(f"{ctx.target}@0x"):
        raise ValueError(f"{old_name} post-apply selector target mismatch")
    try:
        address = int(selector.rsplit("@", 1)[1], 0)
    except ValueError as error:
        raise ValueError(f"{old_name} post-apply selector is invalid") from error
    _binding_digest(row, kind, old_name, address, ctx)
    _manifest_holds(row, kind, old_name, address, ctx)
    mapped = [
        symbol
        for symbol in load_target_symbols(ctx.root, ctx.target)
        if symbol.name == new_name
    ]
    if len(mapped) != 1 or mapped[0].address != address:
        raise ValueError(f"{old_name} post-apply map address changed")
    expected_bindings = set(identity.get("binding_locations", []))
    expected_sources = set(identity.get("source_locations", []))
    actual = ctx.scope(new_name, address=address, kind=kind)
    if set(actual["binding_locations"]) != expected_bindings:
        raise ValueError(f"{old_name} post-apply binding scope does not match report")
    recorded_facts = row["pre_apply"]["facts"]
    recorded_scope = recorded_facts["scope"]
    old_definition = recorded_scope.get("definition")
    if old_definition:
        expected_sources.discard(old_definition)
    expected_destination = recorded_facts.get("destination")
    if actual.get("definition") != expected_destination:
        raise ValueError(f"{old_name} post-apply source destination changed")
    if expected_destination:
        expected_sources.add(expected_destination)
    if set(actual["source_locations"]) != expected_sources:
        raise ValueError(f"{old_name} post-apply source scope does not match report")
    if kind == "data":
        from harness.naming.readiness import canonical_storage

        derived = canonical_storage(ctx.root, ctx.target, address)
        reported = row.get("storage")
        if not isinstance(reported, dict) or any(
            reported.get(field) != derived[field]
            for field in ("kind", "start", "end", "file_offset", "present_in_binary")
        ):
            raise ValueError(f"{old_name} post-apply storage changed")
        if sorted(reported.get("authority", [])) != sorted(derived["authority"]):
            raise ValueError(f"{old_name} post-apply storage authority changed")
    _validation_receipts(row, kind, old_name, address, ctx)


def _validation_receipts(
    row: dict[str, Any], kind: str, old_name: str, address: int, ctx: TargetContext
) -> None:
    selector = f"{ctx.target}@0x{address:08X}"
    records = command_records(
        row.get("post_apply_receipts"), f"{old_name}.post_apply_receipts", ctx.root
    )
    passed = [record["command"] for record in records if record["status"] == "passed"]
    if len(passed) != len(records):
        raise ValueError(f"{old_name} post-apply receipt contains a failed command")
    required = [
        command_text("symbols", "normalize"),
        command_text("symbols", "check"),
        "independent review",
    ]
    native_selectors = None
    if kind == "function":
        required += [command_text("splat"), command_text("build")]
        if row.get("partial_used") is True:
            required += ["partial baseline"]
        else:
            native_selectors = {
                check["selector"] for check in collect_function_checks(ctx.root, row)
            }
    elif "data" in row["pre_apply"]["facts"]:
        consumers = validate_data_shape(row["pre_apply"]["facts"]["data"])["consumers"]
        native_selectors = {consumer["selector"] for consumer in consumers}
        required += [command_text("splat"), command_text("build")]
    if native_selectors is not None:
        required += [
            command_text(tool, native_selector)
            for native_selector in sorted(native_selectors)
            for tool in ("asm-diff", "byte-match")
        ]
    for prefix in required:
        matching = [
            command
            for command in passed
            if command == prefix or command.startswith(prefix + " ")
        ]
        if len(matching) != 1:
            raise ValueError(
                f"{old_name} post-apply receipts require exactly one passed {prefix}"
            )
    for record in records:
        command = record["command"]
        if record.get("target") != ctx.target:
            raise ValueError(f"{old_name} post-apply receipt target mismatch")
        selector_required = command.startswith(
            (
                command_text("asm-diff"),
                command_text("byte-match"),
                "partial baseline",
                "independent review",
            )
        )
        expected_selector = selector
        if native_selectors is not None:
            parts = command.split()
            if tool_id(parts) in {"asm-diff", "byte-match"}:
                selectors = trailing_arguments(parts)
                if not selectors or selectors[0] not in native_selectors:
                    raise ValueError(
                        f"{old_name} receipt does not select a captured consumer"
                    )
                expected_selector = selectors[0]
            elif command.startswith("partial baseline"):
                raise ValueError(f"{kind} consumer partial baselines are unsupported")
            elif (
                command.startswith("independent review")
                and command != f"independent review {selector}"
            ):
                raise ValueError(f"{old_name} independent review selector mismatch")
        if selector_required and record.get("selector") != expected_selector:
            raise ValueError(f"{old_name} post-apply receipt selector mismatch")
        if not selector_required and record.get("selector") is not None:
            raise ValueError(
                f"{old_name} post-apply receipt has an unexpected selector"
            )


def _manifest_holds(
    row: dict[str, Any], kind: str, old_name: str, address: int, ctx: TargetContext
) -> None:
    """The captured tool manifest must still hold after the transaction.

    ``naming_manifest`` re-derives the record from the captured pre-apply
    facts plus the live collision state; scope/storage/reviewed facts in
    the captured record are already proven unchanged by the digest and
    scope checks above, so only the collision result and required checks
    may drift (a new boundary or other proposal can reject the spelling).
    """
    manifest = row.get("manifest")
    if not isinstance(manifest, dict):
        raise ValueError(f"{old_name} post-apply requires the captured manifest")
    expected = naming_manifest(ctx, kind, old_name, row, row["pre_apply"])
    if manifest != expected:
        changed = sorted(
            key
            for key in set(manifest) | set(expected)
            if manifest.get(key) != expected.get(key)
        )
        raise ValueError(f"{old_name} manifest drifted: {changed}")


def _binding_digest(
    row: dict[str, Any], kind: str, old_name: str, address: int, ctx: TargetContext
) -> None:
    """Bind the captured pre-apply facts to the post-apply repository.

    The row carries the immutable, versioned ``pre_apply`` record captured
    before the transaction applied: selector/kind/address, accepted
    statuses, proposed new name, reviewed range, canonical scope, generated
    work, canonical storage, and the reviewed.rz (plus recursive local
    includes) scope digest.  The recorded digest must hash to the recorded
    facts (a versioned, tamper-evident receipt); the captured immutable
    facts must still hold against the current repository; and the applied
    spelling must equal the captured proposal.  Migration paths are checked
    against the post-apply repository by the scope/map checks that follow.
    """

    binding = row.get("pre_apply")
    if not isinstance(binding, dict):
        raise ValueError(
            f"{old_name} post-apply requires the captured pre-apply facts "
            "and digest (pre_apply)"
        )
    version = binding.get("version")
    if version != DIGEST_VERSION:
        raise ValueError(
            f"{old_name} pre-apply facts are version {version}, "
            f"expected v{DIGEST_VERSION}"
        )
    recorded = binding.get("facts")
    if not isinstance(recorded, dict):
        raise ValueError(f"{old_name} pre-apply record requires facts")
    digest = binding.get("digest")
    if not isinstance(digest, str):
        raise ValueError(f"{old_name} pre-apply record requires the digest")
    if not digest.startswith(f"v{DIGEST_VERSION}:"):
        raise ValueError(
            f"{old_name} pre-apply digest is not version v{DIGEST_VERSION}"
        )
    payload = json.dumps(recorded, sort_keys=True, separators=(",", ":"))
    expected = (
        f"v{DIGEST_VERSION}:{hashlib.sha256(payload.encode('utf-8')).hexdigest()}"
    )
    if digest != expected:
        raise ValueError(f"{old_name} pre-apply digest does not match its facts")
    if (
        recorded.get("kind") != kind
        or recorded.get("selector") != f"{ctx.target}@0x{address:08X}"
    ):
        raise ValueError(f"{old_name} pre-apply selector/kind mismatch")
    if recorded.get("new_name") != row.get("new_name"):
        raise ValueError(
            f"{old_name} post-apply new_name differs from the captured proposal"
        )
    captured_status = recorded.get("status")
    if not isinstance(captured_status, dict) or captured_status != READY_STATUS:
        raise ValueError(f"{old_name} pre-apply record is not accepted and ready")
    for field in ("rung_status", "semantic_status", "transaction_status"):
        if row.get(field) != captured_status.get(field):
            raise ValueError(
                f"{old_name} post-apply {field} changed from the captured status"
            )
    captured_range = recorded.get("unchanged_range")
    identity = row.get("identity")
    if (
        not isinstance(identity, dict)
        or identity.get("unchanged_range") != captured_range
        or not range_contains(captured_range, address)
    ):
        raise ValueError(f"{old_name} post-apply reviewed range changed")
    _binding_scope_holds(recorded.get("scope"), old_name, ctx)
    if ctx.required_work(kind, old_name) != recorded.get("work"):
        raise ValueError(f"{old_name} post-apply required work changed")
    if reviewed_scope_digest(ctx.root, ctx.target) != recorded.get("reviewed_digest"):
        raise ValueError(f"{old_name} post-apply reviewed annotations changed")
    if kind == "data":
        from harness.naming.readiness import canonical_storage

        pre_storage = recorded.get("storage")
        post_storage = row.get("storage")
        if pre_storage != post_storage or pre_storage != canonical_storage(
            ctx.root, ctx.target, address
        ):
            raise ValueError(f"{old_name} pre-apply storage changed")
        if "data" in recorded:
            if (
                recorded.get("destination") is not None
                or recorded["scope"].get("definition") is not None
            ):
                raise ValueError("data transactions cannot migrate source files")
            validate_data_preservation(ctx, row, recorded, post_apply=True)


def _binding_scope_holds(recorded: Any, old_name: str, ctx: TargetContext) -> None:
    """The recorded immutable scope must still hold after the transaction.

    The post-apply repository may no longer contain the old spelling, so
    the re-derived old-name scope cannot be recomputed and compared: a
    clean apply rewrites exactly the files the recorded scope covers.
    Instead each recorded binding/source location must still exist and
    must not still carry the old spelling (checked through recursively
    resolved local includes); the recorded scope record itself must be
    intact.  Migration to the new spelling is checked by the map and
    new-name scope checks that follow.
    """
    if not isinstance(recorded, dict):
        raise ValueError(f"{old_name} pre-apply record requires the captured scope")
    locations = sorted(
        set(recorded.get("binding_locations", []))
        | set(recorded.get("source_locations", []))
    )
    if not locations:
        raise ValueError(f"{old_name} post-apply transaction scope changed")
    from harness.domain.includes import local_include_files

    pattern = re.compile(rf"\b{re.escape(old_name)}\b")
    root = ctx.root.resolve()
    files = [root / location for location in locations]
    for path in [*files, *local_include_files(root, files)]:
        if not path.is_file():
            # A metadata-owned lift path may move as part of the spelling
            # transaction; the derived new-name scope below proves its
            # replacement. Missing non-source scope is caught by that equality.
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if pattern.search(text):
            raise ValueError(
                f"old spelling remains: {old_name} in "
                f"{path.relative_to(root).as_posix()}"
            )


__all__ = ["validate"]
