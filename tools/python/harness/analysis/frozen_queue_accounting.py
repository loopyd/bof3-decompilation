"""Read-only accounting of the unchanged, externally pinned S3.3 frozen five."""

from __future__ import annotations

import copy
import hashlib
import re
from contextlib import closing
from pathlib import Path
from typing import Literal, TypedDict

from harness.analysis import index
from harness.common.digests import digest
from harness.macros import application as macro_application_review
from harness.macros.opportunities import macro_opportunities_payload
from harness.naming import audit
from harness.naming import terminal as terminal_review
from harness.naming.inputs import keys, load
from harness.naming.namespace import (
    canonical_evidence_root,
    reset_evidence_root,
    set_evidence_root,
)
from harness.types import application as type_application_review
from harness.types.queries import type_candidates_payload

from ..domain.claims import manifest_header_paths
from ..domain.ids import parse_function_id
from ..domain.manifests import load_target_manifests
from ..domain.receipts import reset_receipt_root, set_receipt_root
from ..domain.registry import resolve_function
from .rev_queries import describe_payload

PILOT_SHA256 = "bb68a07c5fe271005251d11634b6ed4496a28418d961193304ad39f00719c218"
TARGET = "emi/battle/battle/15"
IDS = (
    f"{TARGET}@800A3638",
    f"{TARGET}@80096994",
    f"{TARGET}@1F800044:storage",
    f"{TARGET}@1F800044:aggregate_region",
    "exact_group:8e1ad03b4ba92303",
)
REPORT = "out/reviews/plan-audit-naming/emi__battle__battle__15.json"
Outcome = Literal["accepted", "noop", "blocked"]


class Blocker(TypedDict):
    owner: str
    reason: str
    next_action: str


class FunctionProof(TypedDict):
    bundle: Path
    expected_bundle_sha256: str


class TerminalProof(TypedDict):
    parent: Path
    expected_parent_digest: str


class ApplicationProof(TypedDict):
    envelope: Path
    expected_envelope_digest: str


Proof = FunctionProof | TerminalProof | ApplicationProof


class FrozenEntryRecord(TypedDict):
    id: str
    claim: Outcome
    proof: Proof | None
    blocker: Blocker | None


class AccountedEntry(TypedDict):
    id: str
    route: str
    historical_skip: bool
    outcome: Outcome
    proof: Proof | None
    blocker: Blocker | None


class Counts(TypedDict):
    accepted: int
    noop: int
    blocked: int
    historical_skip: int


class OverlapGroup(TypedDict):
    id: str
    entries: list[str]
    outcome: Outcome


class FrozenQueueAccounting(TypedDict):
    pilot_sha256: str
    accounted: bool
    entries: list[AccountedEntry]
    counts: Counts
    overlap_groups: list[OverlapGroup]
    full_report: dict
    campaign_complete: bool
    production_complete: Literal[False]


def _path(value: Path, *, root: Path | None = None) -> Path:
    if not isinstance(value, Path):
        raise ValueError("artifact reference must be a canonical absolute Path")
    path = canonical_evidence_root(value)
    if root is not None and not path.is_relative_to(root):
        raise ValueError("frozen reference escapes repository root")
    if not path.is_file():
        raise ValueError(f"retained file missing: {path}")
    return path


def _sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _pin(value: str, *, versioned: bool = False) -> None:
    pattern = r"v1:[0-9a-f]{64}" if versioned else r"[0-9a-f]{64}"
    if not isinstance(value, str) or re.fullmatch(pattern, value) is None:
        raise ValueError("invalid external digest pin")


def _fresh(root: Path, pilot: Path, frozen: dict, report: Path) -> None:
    # ponytail: unchanged S3.3 baseline only; mutations require a separately
    # reviewed fresh native input, not migration or historical receipt rebinding.
    pins = {pilot: PILOT_SHA256, report: frozen["report_sha256"]}
    pins[index.index_path(root)] = frozen["index_sha256"]
    for fingerprint in frozen["fingerprints"].values():
        pins.update({root / p: h for p, h in fingerprint["source_inputs"].items()})
        target = fingerprint["indexed_target"]
        for kind in ("binary", "snapshot"):
            pins[root / target[kind]] = target[kind + "_sha256"]
    for path, expected in pins.items():
        if _sha(_path(path, root=root)) != expected:
            raise ValueError(
                f"frozen baseline drift; parent must review fresh input: {path}"
            )
    with closing(index.connect(root)) as connection:
        manifests = load_target_manifests(root)
        for entry in frozen["entries"][:2]:
            current = describe_payload(
                connection,
                parse_function_id(entry["id"]),
                root=root,
                manifests=manifests,
                limit=20,
            )
            if current != entry["query"]:
                raise ValueError("frozen naming selector changed")
        candidates = {
            row["id"]: row
            for row in type_candidates_payload(
                connection,
                target=TARGET,
                status=None,
                limit=0,
            )
        }
        if any(candidates.get(e["id"]) != e["query"] for e in frozen["entries"][2:4]):
            raise ValueError("frozen type concern changed")
        for target, field in ((TARGET, "query"), (None, "cross_target_report_only")):
            opportunities = macro_opportunities_payload(
                connection,
                root,
                target=target,
                kind="exact_group",
                limit=0,
            )
            selected = [row for row in opportunities if row["id"] == IDS[4]]
            if selected != [frozen["entries"][4][field]]:
                raise ValueError("frozen macro membership changed")


def _application(root: Path, entry: dict, proof: dict) -> None:
    envelope = load(proof["envelope"])
    before = copy.deepcopy(envelope)
    owner = (
        type_application_review if entry["id"] in IDS[2:4] else macro_application_review
    )
    result = owner.verify_reviewed_application(
        root, envelope, proof["expected_envelope_digest"]
    )
    if (
        envelope != before
        or result.get("accepted") is not True
        or result.get("target") != TARGET
        or result.get("digest") != proof["expected_envelope_digest"]
    ):
        raise ValueError("owner acceptance binding differs")
    manifest = envelope["application"]["manifest"]
    if manifest["target"] != TARGET or manifest["targets"] != [TARGET]:
        raise ValueError("application expands frozen target scope")
    if entry["id"] in IDS[2:4]:
        rows = manifest["reviewed_candidates"]
        if (
            sorted(manifest["request"]["index_candidate_ids"]) != sorted(IDS[2:4])
            or sorted(row["index_id"] for row in rows) != sorted(IDS[2:4])
            or any(
                row["candidate"]["target"] != TARGET
                or row["candidate"]["address"] != 0x1F800044
                for row in rows
            )
        ):
            raise ValueError("type envelope does not bind both frozen concerns")
    else:
        reviewed = manifest["reviewed_opportunity"]
        members = sorted(row["function"] for row in entry["query"]["members"])
        sources = set()
        for member in members:
            source = resolve_function(root, member).source
            if source is None:
                raise ValueError("frozen macro member has no claimed source")
            sources.add(source.relative_to(root).as_posix())
        headers = {
            p.relative_to(root).as_posix()
            for p in manifest_header_paths(root, load_target_manifests(root)[TARGET])
        }
        if (
            reviewed["candidate_id"] != entry["id"]
            or reviewed["candidate_fingerprint"]
            != digest(entry["cross_target_report_only"])
            or reviewed["declared_targets"] != [TARGET]
            or not set(reviewed["owners"]) <= sources | headers
            or not set(manifest["allowed_paths"]) <= sources | headers
            or sorted(
                parse_function_id(row["selector"]).value
                for row in manifest["affected_functions"]
            )
            != members
        ):
            raise ValueError("macro envelope does not bind frozen fingerprint/members")


def _positive(root: Path, entry: dict, record: FrozenEntryRecord, report: Path) -> None:
    claim = record["claim"]
    proof: dict = dict(record["proof"]) if isinstance(record["proof"], dict) else {}
    identity = entry["id"]
    if identity == IDS[0] and claim == "accepted":
        reference, pin, versioned = "bundle", "expected_bundle_sha256", False
    elif identity == IDS[1] and claim == "noop":
        reference, pin, versioned = "parent", "expected_parent_digest", False
    elif identity in IDS[2:] and claim == "accepted":
        reference, pin, versioned = "envelope", "expected_envelope_digest", True
    else:
        raise ValueError("unsupported frozen claim/owner route")
    keys(proof, f"{reference} {pin}")
    _pin(proof[pin], versioned=versioned)
    path = _path(proof[reference])
    before = _sha(path)
    if reference == "bundle":
        if before != proof[pin]:
            raise ValueError("bundle differs from external pin")
        result = audit.verify(
            root,
            TARGET,
            load(report),
            entry["report_row"],
            report_path=report,
            post_apply_receipts=path,
        )
        if (
            result.get("applied") is not True
            or type(result.get("rows")) is not int
            or result["rows"] != 1
            or result.get("target") != TARGET
            or result.get("transaction") != entry["report_row"]
        ):
            raise ValueError("FUNCTION owner returned mismatched acceptance")
    elif reference == "parent":
        result = terminal_review.verify_terminal(
            root, TARGET, report, entry["report_row"], path, proof[pin]
        )
        if (
            result.get("selected_row_accepted") is not True
            or result.get("target") != TARGET
            or result.get("transaction") != entry["report_row"]
            or result.get("parent_digest") != proof[pin]
        ):
            raise ValueError("terminal owner returned mismatched acceptance")
    else:
        _application(root, entry, proof)
    if _sha(_path(path)) != before:
        raise ValueError("retained proof changed during verification")


def account_frozen_queue(
    root: Path,
    pilot: Path,
    expected_pilot_sha256: str,
    *,
    records: list[FrozenEntryRecord],
    report_path: Path,
    evidence_root: Path,
) -> FrozenQueueAccounting:
    """Check five explicit outcomes; accounting is neither application nor completion."""
    root = canonical_evidence_root(root)
    pilot, report = _path(pilot, root=root), _path(report_path, root=root)
    _pin(expected_pilot_sha256)
    if expected_pilot_sha256 != PILOT_SHA256 or _sha(pilot) != expected_pilot_sha256:
        raise ValueError("pilot differs from pinned frozen-five input")
    if report != root / REPORT:
        raise ValueError("wrong frozen report path")
    frozen = load(pilot)
    if [entry["id"] for entry in frozen["entries"]] != list(IDS):
        raise ValueError("wrong frozen five-entry membership")
    records = copy.deepcopy(records)
    if not isinstance(records, list) or len(records) != 5:
        raise ValueError("exactly five entry records required")
    for record in records:
        keys(record, "id claim proof blocker")
        if not isinstance(record["id"], str) or record["claim"] not in (
            "accepted",
            "noop",
            "blocked",
        ):
            raise ValueError("invalid frozen ID or claim")
        if record["claim"] == "blocked":
            blocker = record["blocker"]
            keys(blocker, "owner reason next_action")
            if (
                not isinstance(blocker, dict)
                or record["proof"] is not None
                or any(
                    not isinstance(v, str) or not v.strip() for v in blocker.values()
                )
            ):
                raise ValueError("blocked record requires attribution and no proof")
        elif record["blocker"] is not None:
            raise ValueError("positive claim cannot carry a blocker")
    by_id = {record["id"]: record for record in records}
    if len(by_id) != 5 or set(by_id) != set(IDS):
        raise ValueError("missing, duplicate or extra frozen entry IDs")
    pair = [by_id[identity] for identity in IDS[2:4]]
    if any(r["claim"] != "blocked" for r in pair) and not (
        all(r["claim"] == "accepted" for r in pair)
        and pair[0]["proof"] == pair[1]["proof"]
    ):
        raise ValueError("linked pair requires the same accepted type envelope")
    namespace_token = set_evidence_root(canonical_evidence_root(evidence_root))
    try:
        receipt_token = set_receipt_root(evidence_root)
        try:
            _fresh(root, pilot, frozen, report)
            references = {}
            for record in records:
                if isinstance(record["proof"], dict):
                    for value in record["proof"].values():
                        if isinstance(value, Path):
                            path = _path(value)
                            references[path] = _sha(path)
            for entry in frozen["entries"]:
                record = by_id[entry["id"]]
                if record["claim"] != "blocked":
                    _positive(root, entry, record, report)
            try:
                full = audit.validate(
                    root, TARGET, load(report), transaction=None, report_path=report
                )
                if type(full.get("complete")) is not bool:
                    raise ValueError("full-report owner returned nonboolean complete")
            except ValueError as error:
                full = {"complete": False, "blocker": str(error)}
            _fresh(root, pilot, frozen, report)
            if any(_sha(_path(path)) != sha for path, sha in references.items()):
                raise ValueError("retained proof changed during accounting")
        finally:
            reset_receipt_root(receipt_token)
    finally:
        reset_evidence_root(namespace_token)
    entries = [
        AccountedEntry(
            id=e["id"],
            route=e["route"],
            historical_skip=e["id"] == IDS[0],
            outcome=by_id[e["id"]]["claim"],
            proof=by_id[e["id"]]["proof"],
            blocker=by_id[e["id"]]["blocker"],
        )
        for e in frozen["entries"]
    ]
    counts = Counts(accepted=0, noop=0, blocked=0, historical_skip=1)
    for entry in entries:
        counts[entry["outcome"]] += 1
    return FrozenQueueAccounting(
        pilot_sha256=expected_pilot_sha256,
        accounted=True,
        entries=entries,
        counts=counts,
        overlap_groups=[
            OverlapGroup(
                id=f"{TARGET}@1F800044",
                entries=list(IDS[2:4]),
                outcome=pair[0]["claim"],
            )
        ],
        full_report=full,
        campaign_complete=counts["blocked"] == 0,
        production_complete=False,
    )
