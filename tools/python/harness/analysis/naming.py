"""Consolidated bof3.naming-audit/v3 row validation and immutable transaction facts.

Analysis-specific derivation stays here: transaction scope, canonical
storage, and generated required work come from ``naming_readiness``; the
generic report/receipt/rung structural parsing lives in
``domain.naming_facts`` so no domain module ever imports analysis.
"""

from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import TYPE_CHECKING, Any, Mapping

from ..domain.claims import (
    manifest_source_paths,
    resolve_source_for_paths,
)
from ..domain.naming_debt import address_of, collect_naming_debt
from ..domain.naming_facts import (
    corroborators as _corroborators,
)
from ..domain.naming_facts import (
    metadata_state as _metadata_state_uncached,
)
from ..domain.naming_facts import (
    name_terms_v3 as _name_terms_v3,
)
from ..domain.naming_facts import (
    optional_work as _optional_work,
)
from ..domain.naming_facts import (
    parse_range as _parse_range,
)
from ..domain.naming_facts import (
    required_work as _required_work,
)
from ..domain.naming_facts import (
    row_observation_ids as _row_observation_ids,
)
from ..domain.naming_facts import (
    row_rungs as _row_rungs,
)
from ..domain.naming_facts import (
    scope_equality as _scope_equality,
)
from ..domain.naming_facts import (
    status_pair as _status_pair,
)
from ..domain.registry import payload_end_for, resolve_target
from ..domain.tags import parse_progress_tags
from .naming_readiness import canonical_storage
from .naming_readiness import (
    RequiredWorkSnapshot,
    required_work_items,
    transaction_scope,
)
from .naming_reviewed import (
    expected_reviewed_digest,
    reviewed_annotations,
    reviewed_scope_digest as _reviewed_scope_digest,
)

if TYPE_CHECKING:
    from ..domain.manifests import TargetManifest

CAMEL_CASE = re.compile("[a-z][A-Za-z0-9]*\\Z")
TERMINAL = {"proposed", "exhausted", "blocked"}
_RANGE = re.compile(r"(0x[0-9A-Fa-f]{8})\.\.(0x[0-9A-Fa-f]{8})")

DIGEST_VERSION = 1
MANIFEST_VERSION = 1
MANIFEST_SCHEMA = f"bof3.naming-transaction/v{MANIFEST_VERSION}"
SCHEMA_V2 = "bof3.naming-audit/v2"
SCHEMA_V3 = "bof3.naming-audit/v3"
READY_STATUS = {
    "rung_status": "proposed",
    "semantic_status": "accepted",
    "transaction_status": "ready",
}


class TargetContext:
    """Per-target derived facts, cached across rows."""

    def __init__(
        self,
        root: Path,
        target: str,
        manifest: TargetManifest,
        *,
        work_snapshot: RequiredWorkSnapshot | None = None,
        source_metadata: Mapping[int, tuple[Path, Any]] | None = None,
        payload_end: int | None = None,
    ):
        self.root = root
        self.target = target
        self.manifest = manifest
        self.payload_end = (
            payload_end_for(root, manifest) if payload_end is None else payload_end
        )
        self.work_snapshot = work_snapshot
        self.source_metadata = source_metadata
        self.work_cache: dict[tuple[str, str], list[dict[str, str]]] = {}
        self.scope_cache: dict[str, dict] = {}

    def partial(self, name: str) -> bool:
        """Return cached repository-derived partial status."""

        return _metadata_state(self, name)[2]

    def required_work(self, kind: str, name: str) -> list[dict[str, str]]:
        key = (kind, name)
        if key not in self.work_cache:
            if self.work_snapshot is not None:
                self.work_cache[key] = self.work_snapshot.items(address_of(name), kind)
            else:
                self.work_cache[key] = required_work_items(
                    self.root, self.target, address_of(name), kind
                )
        return self.work_cache[key]

    def scope(
        self, name: str, *, address: int | None = None, kind: str | None = None
    ) -> dict:
        key = f"{name}:{address}:{kind}"
        if key not in self.scope_cache:
            self.scope_cache[key] = transaction_scope(
                self.root, self.target, name, address=address, kind=kind
            )
        return self.scope_cache[key]


def inventory_expected(
    root: Path, target: str, manifests: dict[str, TargetManifest]
) -> set[tuple[str, str]]:
    debt = collect_naming_debt(root, manifests)
    return {
        (kind, value.split(":", 1)[1])
        for kind, values in (("function", debt.raw_functions), ("data", debt.raw_data))
        for value in values
        if value.startswith(f"{target}:")
    }


def partial_status(root: Path, manifest: TargetManifest, name: str) -> bool:
    """Derive partial_lift from the metadata-resolved source, never the report."""
    address = address_of(name)
    source = None
    try:
        resolved = resolve_target(root, manifest.id.value)
        source = resolve_source_for_paths(resolved.source_paths, address)
    except (FileNotFoundError, ValueError, RuntimeError):
        source = None
    if source is None:
        return False
    try:
        progress = parse_progress_tags(source.read_text(encoding="utf-8"))
    except ValueError:
        return False
    return progress is not None and progress[0] == "partial"


def _metadata_state(ctx: TargetContext, name: str) -> tuple[bool, str, bool]:
    """Return cached source metadata validity and partial status for one row."""

    if ctx.source_metadata is None:
        metadata_ok, detail = _metadata_state_uncached(ctx.root, ctx.manifest, name)
        return metadata_ok, detail, partial_status(ctx.root, ctx.manifest, name)
    source, progress = ctx.source_metadata.get(address_of(name), (None, None))
    if source is None:
        return True, "no claimed source", False
    if isinstance(progress, ValueError):
        return False, f"{source}: {progress}", False
    return True, "metadata canonical", progress is not None and progress[0] == "partial"


def reviewed_scope_digest(root: Path, target: str) -> str | None:
    """Compatibility export for naming-audit validation."""
    return _reviewed_scope_digest(root, target)


def pre_apply_facts(ctx: TargetContext, kind: str, name: str, new_name: str) -> dict:
    address = address_of(name)
    facts: dict = {
        "selector": f"{ctx.target}@0x{address:08X}",
        "kind": kind,
        "address": f"0x{address:08X}",
        "old_name": name,
        "new_name": new_name,
        "scope": ctx.scope(name, address=address, kind=kind),
        "work": ctx.required_work(kind, name),
        "reviewed": reviewed_annotations(ctx.root, ctx.target),
        "reviewed_digest": expected_reviewed_digest(
            ctx.root, ctx.target, name, new_name
        ),
    }
    if kind == "data":
        facts["storage"] = canonical_storage(ctx.root, ctx.target, address)
    try:
        destination = resolve_source_for_paths(
            manifest_source_paths(ctx.root, ctx.manifest), address
        )
    except (ValueError, OSError):
        destination = None
    facts["destination"] = (
        destination.with_name(f"{new_name}{destination.suffix}")
        .relative_to(ctx.root)
        .as_posix()
        if destination
        else None
    )
    return facts


def pre_apply(ctx: TargetContext, kind: str, name: str, row: dict[str, object]) -> dict:
    """Digest the immutable pre-apply facts plus the accepted row statuses.

    Captured once, before the transaction applies; the report stores it and
    ``--post-apply`` fails when the recorded facts no longer hash to the
    recorded digest.  Scope/work/storage/reviewed facts must be derived
    before the apply step because the apply step rewrites exactly the
    spelling-bearing files inside them.
    """
    address = address_of(name)
    new_name = row.get("new_name")
    if not isinstance(new_name, str) or not new_name:
        raise ValueError(f"{name} pre-apply requires new_name")
    facts = pre_apply_facts(ctx, kind, name, new_name)
    identity = row.get("identity")
    range_value = (
        identity.get("unchanged_range") if isinstance(identity, dict) else None
    )
    if not isinstance(range_value, str) or _RANGE.fullmatch(range_value) is None:
        raise ValueError(
            f"{name} pre-apply requires identity.unchanged_range "
            "'0xXXXXXXXX..0xYYYYYYYY' containing the symbol"
        )
    start, end = _parse_range(range_value, name)
    if not start <= address < end:
        raise ValueError(f"{name} unchanged_range must contain the symbol address")
    facts["unchanged_range"] = range_value
    facts["status"] = {
        "rung_status": row.get("rung_status"),
        "semantic_status": row.get("semantic_status"),
        "transaction_status": row.get("transaction_status"),
    }
    payload = json.dumps(facts, sort_keys=True, separators=(",", ":"))
    return {
        "version": DIGEST_VERSION,
        "digest": f"v{DIGEST_VERSION}:{hashlib.sha256(payload.encode('utf-8')).hexdigest()}",
        "facts": facts,
    }


def live_collision(ctx: TargetContext, address: int, new_name: str) -> dict | None:
    """Re-run the cross-target collision matrix against the current repo."""
    from ..domain import load_target_manifests
    from ..domain.identity import (
        propose_collision,
        reviewed_function_identities,
    )

    manifests = load_target_manifests(ctx.root)
    identities = [
        identity
        for target, manifest in manifests.items()
        for identity in reviewed_function_identities(ctx.root, target, manifest)
    ]
    finding = propose_collision(identities, ctx.target, address, new_name)
    return (
        None
        if finding is None
        else {
            "rule": finding.rule,
            "verdict": finding.verdict,
            "detail": finding.detail,
        }
    )


def naming_manifest(
    ctx: TargetContext, kind: str, name: str, row: dict[str, object], binding: dict
) -> dict:
    """Tool-owned versioned naming transaction manifest.

    Extends the immutable ``pre_apply`` record with the collision result
    and the required post-apply checks so the apply step is fully
    specified by the tool: no agent prose decides scope, collisions,
    storage, or which gates must pass.  ``verify`` re-derives and
    compares it against the current repository.
    """
    address = address_of(name)
    collision = (
        live_collision(ctx, address, str(row.get("new_name")))
        if kind == "function"
        else None
    )
    manifest = {
        "schema": MANIFEST_SCHEMA,
        "transaction": f"{kind}:{name}",
        "pre_apply_digest": binding["digest"],
        "inventory": {
            "selector": f"{ctx.target}@0x{address:08X}",
            "kind": kind,
            "address": f"0x{address:08X}",
            "old_name": name,
            "new_name": row.get("new_name"),
        },
        "scope": binding["facts"]["scope"],
        "reviewed": binding["facts"]["reviewed"],
        "reviewed_digest": binding["facts"]["reviewed_digest"],
        "storage": binding["facts"].get("storage"),
        "range": binding["facts"]["unchanged_range"],
        "baseline": binding["facts"]["status"],
        "work": binding["facts"]["work"],
        "collision": collision,
        "required_checks": sorted(
            (
                ["bin/symbols check"]
                + (["bin/splat TARGET"] if kind == "function" else [])
                + ["bin/naming-audit verify"]
            )
        ),
    }
    return manifest


def validate_row_v3(row: object, kind: str, name: str, ctx: TargetContext) -> bool:
    """Validate one v3 row; return whether it is blocked."""
    if not isinstance(row, dict):
        raise ValueError(f"rows must be objects for {name}")
    state = row.get("rung_status")
    if state not in TERMINAL:
        raise ValueError(f"invalid rung_status for {kind}:{name}")
    outside = not ctx.manifest.load_address <= address_of(name) < ctx.payload_end
    if row.get("outside_payload") is not outside:
        raise ValueError(
            f"outside_payload must be {str(outside).lower()} for {kind}:{name}"
        )
    metadata_ok, metadata_detail, actual_partial = _metadata_state(ctx, name)
    if row.get("partial_used") is not actual_partial:
        raise ValueError(f"partial_used must match source metadata for {kind}:{name}")
    if not metadata_ok and state != "blocked":
        raise ValueError(
            f"{name} blocked by malformed lift metadata: {metadata_detail}"
        )
    rung_failed = _row_rungs(row, kind, outside, ctx.root, name)
    generated = ctx.required_work(kind, name)
    work_blocked = _required_work(row, name, ctx.root, generated)
    _optional_work(row, name, generated, work_blocked or rung_failed)
    blocked = work_blocked or rung_failed or (not metadata_ok)
    if state == "blocked":
        if (
            not blocked
            or not isinstance(row.get("smallest_repair"), str)
            or (not row["smallest_repair"].strip())
        ):
            raise ValueError(f"blocked row requires failed rung for {kind}:{name}")
    elif blocked:
        raise ValueError(
            f"failed rung or open work requires blocked status for {kind}:{name}"
        )
    for field in ("interpretation", "authority"):
        if not isinstance(row.get(field), str) or not row[field].strip():
            raise ValueError(f"missing {field} for {kind}:{name}")
    if row.get("pending_commands"):
        raise ValueError(f"pending commands remain for {kind}:{name}")
    if state == "proposed":
        observation_ids = _row_observation_ids(row, name)
        corroborators = _corroborators(row, name, observation_ids)
        new_name = str(row.get("new_name", ""))
        if not CAMEL_CASE.fullmatch(new_name):
            raise ValueError(f"new_name must be camelCase for {name}")
        _name_terms_v3(row, new_name, corroborators)
        if row.get("new_name") != row.get("identity", {}).get("new"):
            raise ValueError(f"{name} new_name must equal identity.new")
        if kind == "function":
            from ..domain import load_target_manifests
            from ..domain.identity import (
                propose_collision,
                reviewed_function_identities,
            )

            manifests = load_target_manifests(ctx.root)
            identities = [
                identity
                for target, manifest in manifests.items()
                for identity in reviewed_function_identities(ctx.root, target, manifest)
            ]
            collision = propose_collision(
                identities, ctx.target, address_of(name), new_name
            )
            if collision is not None:
                raise ValueError(str(collision))
        _scope_equality(row, name, ctx.target, ctx.scope(name))
        if kind == "data":
            _storage_equality(row, name, ctx)
        _status_pair(row, name, blocked)
    elif (
        not isinstance(row.get("missing_fact"), str) or not row["missing_fact"].strip()
    ):
        raise ValueError(f"missing missing_fact for {kind}:{name}")
    return blocked


def _storage_equality(row: dict[str, object], name: str, ctx: "TargetContext") -> None:
    derived = canonical_storage(ctx.root, ctx.target, address_of(name))
    reported = row.get("storage")
    if not isinstance(reported, dict):
        raise ValueError(f"proposed data requires exact storage for {name}")
    for field in ("kind", "start", "end", "file_offset"):
        if reported.get(field) != derived[field]:
            raise ValueError(f"{name}.storage.{field} must equal canonical storage")
    if reported.get("present_in_binary") is not derived["present_in_binary"]:
        raise ValueError(
            f"{name}.storage.present_in_binary must equal canonical storage"
        )
    if sorted(reported.get("authority", [])) != sorted(derived["authority"]):
        raise ValueError(f"{name}.storage.authority must equal canonical storage")


__all__ = [
    "DIGEST_VERSION",
    "MANIFEST_SCHEMA",
    "SCHEMA_V2",
    "SCHEMA_V3",
    "TargetContext",
    "inventory_expected",
    "live_collision",
    "naming_manifest",
    "partial_status",
    "pre_apply",
    "pre_apply_facts",
    "READY_STATUS",
    "reviewed_annotations",
    "validate_row_v3",
]
