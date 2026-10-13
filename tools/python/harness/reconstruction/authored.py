"""Evaluator-only authored references and the seven corpus exclusions."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from harness.domain.ids import normalize_target_id, parse_function_id
from harness.reconstruction import contract
from harness.reconstruction.baseline import MemberFacts

EXCLUSION_REASON_CODES = {
    "Splat interval differs from native function extent": "extent-mismatch",
    "reference native byte check failed or incomplete": "native-byte-check",
}

UNRESOLVED_CATEGORIES = (
    "initialized_data",
    "zero_initialized",
    "relocations",
    "external_bindings",
)


@dataclass(frozen=True)
class AuthoredMember:
    """One member's evaluator-only authored source and target maps."""

    facts: MemberFacts
    symbol: str | None
    signature: str
    source: str
    source_bytes: bytes
    maps: tuple[tuple[str, bytes], ...]


@dataclass(frozen=True)
class ExclusionRecord:
    """One reference-boundary exclusion, kept outside admitted membership."""

    case_id: str
    key: str
    target: str
    address: int
    size: int
    reason: str
    reason_code: str


def authored_members(
    root: Path, baseline: dict, members: list[MemberFacts]
) -> list[AuthoredMember]:
    """Read corpus references and verify their current source and map pins."""

    references = baseline.get("references")
    contract.require(
        isinstance(references, dict)
        and set(references) == {fact.case_id for fact in members},
        "corpus references do not match the admitted member set",
    )
    map_cache: dict[str, tuple[str, bytes]] = {}
    authored: list[AuthoredMember] = []
    for fact in members:
        row = references[fact.case_id]
        contract.require(
            isinstance(row, dict), f"authored reference is not an object: {fact.key}"
        )
        name = row.get("name")
        signature = row.get("signature")
        source = row.get("source")
        contract.require(
            isinstance(name, str) and 0 < len(name) <= 256,
            f"authored name is missing or too long for {fact.key}",
        )
        contract.require(
            isinstance(signature, str) and 0 < len(signature) <= 512,
            f"authored signature is missing or too long for {fact.key}",
        )
        source = contract.validate_rel_path(source)
        contract.require(
            len(source) <= 256, f"authored source locator is too long for {fact.key}"
        )
        source_bytes = contract.read_verified(
            root / source, row.get("source_sha256"), f"authored source {source}"
        )
        maps: list[tuple[str, bytes]] = []
        for key in ("manifest", "splat"):
            relative = contract.validate_rel_path(row.get(key))
            pin = contract.validate_sha256(row.get(f"{key}_sha256"), f"map {relative}")
            if relative not in map_cache:
                map_cache[relative] = (
                    pin,
                    contract.read_verified(
                        root / relative, pin, f"target map {relative}"
                    ),
                )
            recorded_pin, data = map_cache[relative]
            contract.require(
                recorded_pin == pin, f"target map pins disagree: {relative}"
            )
            maps.append((relative, data))
        authored.append(
            AuthoredMember(
                facts=fact,
                symbol=name if contract.TOKEN.fullmatch(name) is not None else None,
                signature=signature,
                source=source,
                source_bytes=source_bytes,
                maps=tuple(maps),
            )
        )
    contract.require(
        len(authored) == contract.ADMITTED, f"authored member count is {len(authored)}"
    )
    return authored


def exclusions(
    root: Path, baseline: dict, members: list[MemberFacts]
) -> list[ExclusionRecord]:
    """Validate the seven corpus exclusions and keep them outside membership."""

    rows = baseline.get("exclusions")
    contract.require(isinstance(rows, list), "corpus exclusions must be a list")
    admitted_keys = {fact.key for fact in members}
    admitted_cases = {fact.case_id for fact in members}
    case_ids: set[str] = set()
    keys: set[str] = set()
    records: list[ExclusionRecord] = []
    for row in rows:
        contract.require(isinstance(row, dict), "corpus exclusion is not an object")
        case_id = contract.validate_ident(row.get("case_id"), "exclusion case id")
        contract.require(
            case_id not in case_ids, f"duplicate exclusion case id: {case_id}"
        )
        contract.require(
            case_id not in admitted_cases, f"exclusion is admitted: {case_id}"
        )
        try:
            function = parse_function_id(str(row.get("selector")))
            target = normalize_target_id(str(row.get("target")))
        except (ValueError, TypeError) as error:
            raise contract.PreparationRefused(
                f"exclusion identity is invalid: {case_id} ({error})"
            ) from error
        key = function.value
        contract.require(key not in keys, f"duplicate exclusion key: {key}")
        contract.require(key not in admitted_keys, f"exclusion key is admitted: {key}")
        contract.require(
            function.target.value == target.value,
            f"exclusion selector and target disagree: {case_id}",
        )
        address = row.get("address")
        contract.require(
            isinstance(address, int)
            and not isinstance(address, bool)
            and address == function.address,
            f"exclusion selector and address disagree: {case_id}",
        )
        size = row.get("size")
        contract.require(
            isinstance(size, int) and not isinstance(size, bool) and size > 0,
            f"exclusion size is invalid: {case_id}",
        )
        reason = row.get("reason")
        contract.require(
            isinstance(reason, str) and 0 < len(reason) <= 256,
            f"exclusion reason is invalid: {case_id}",
        )
        reason_code = EXCLUSION_REASON_CODES.get(reason)
        contract.require(
            reason_code is not None and reason_code == row.get("reason_code"),
            f"exclusion reason code differs: {case_id}",
        )
        case_ids.add(case_id)
        keys.add(key)
        records.append(
            ExclusionRecord(
                case_id=case_id,
                key=key,
                target=target.value,
                address=address,
                size=size,
                reason=reason,
                reason_code=reason_code,
            )
        )
    contract.require(
        len(records) == contract.EXCLUSIONS, f"exclusion count is {len(records)}"
    )
    return sorted(records, key=lambda record: record.case_id)
