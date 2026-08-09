"""TOML-backed target manifests."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
import re
import tomllib
from typing import Any

from .ids import TargetId, normalize_target_id

_SHA256 = re.compile(r"[0-9a-f]{64}")


@dataclass(frozen=True)
class SectionPlacement:
    function: int
    section: str
    address: int
    size: int


@dataclass(frozen=True)
class CompanionStaticCall:
    caller_address: int
    target_address: int


@dataclass(frozen=True)
class CompanionAbi:
    target_address: int
    prototype: str
    evidence: str


@dataclass(frozen=True)
class CompanionOverlay:
    target: TargetId
    disc_id: str
    payload_sha256: str
    load_address: int
    size: int
    static_calls: tuple[CompanionStaticCall, ...]
    evidence: str
    abi: CompanionAbi | None = None


@dataclass(frozen=True)
class TargetManifest:
    id: TargetId
    disc_id: str
    kind: str
    source_dir: str
    binary: str
    splat: str
    load_address: int
    psyq_space: str = "slus"
    # Explicit target-qualified source claims. ``sources`` are authored lift
    # translation units, ``support_sources`` generated/helper units the build
    # must compile, and ``headers`` target-private headers. Claims may point
    # outside ``source_dir`` (semantic ``src/bof3/<class>/`` folders); when a
    # target declares no claims, ``source_dir`` remains the inventory root
    # (legacy compatibility mode).
    sources: tuple[str, ...] = ()
    support_sources: tuple[str, ...] = ()
    headers: tuple[str, ...] = ()
    # Generated PsyQ weak-binding source (relative to the repo root).  A
    # migrated target (explicit ``sources``/``support_sources``) must declare
    # this so ``bin/symbols psyq-bindings`` never guesses an output path;
    # unmigrated targets keep the legacy ``source_dir/symbols/psyq.c``
    # location.
    psyq_source: str = ""
    libraries: dict[str, tuple[str, ...]] = field(default_factory=dict)
    library_confidence: dict[str, str] = field(default_factory=dict)
    library_evidence: dict[str, tuple[str, ...]] = field(default_factory=dict)
    section_placements: dict[int, tuple[SectionPlacement, ...]] = field(
        default_factory=dict
    )
    companions: tuple[CompanionOverlay, ...] = ()

    @property
    def has_explicit_sources(self) -> bool:
        """True when this target owns its build inputs by explicit source claim.

        Only ``sources``/``support_sources`` activate the explicit source
        inventory. A header-only claim independently overrides the private
        header list while sources keep the legacy ``source_dir`` inventory.
        """

        return bool(self.sources or self.support_sources)

    def __post_init__(self) -> None:
        if self.kind not in {"executable", "emi"}:
            raise ValueError(f"unsupported target kind: {self.kind}")
        if self.psyq_space not in {"slus", "logo"}:
            raise ValueError(f"unsupported psyq space: {self.psyq_space}")
        if self.companions and self.kind != "emi":
            raise ValueError("only EMI targets may declare companion overlays")


def _load_toml(path: Path) -> dict[str, Any]:
    with path.open("rb") as stream:
        return tomllib.load(stream)


def _parse_companions(raw: dict[str, Any], caller: TargetId) -> tuple[CompanionOverlay, ...]:
    values = raw.get("companion_overlays", [])
    if not isinstance(values, list):
        raise ValueError("companion_overlays must be an array of tables")
    companions: list[CompanionOverlay] = []
    seen: set[str] = set()
    for value in values:
        if not isinstance(value, dict):
            raise ValueError("companion overlay must be a table")
        target = normalize_target_id(str(value["target"]))
        disc_id = str(value["disc_id"])
        if target.kind != "emi" or normalize_target_id(disc_id).value != target.value:
            raise ValueError(f"invalid companion overlay identity: {disc_id}")
        if target.value == caller.value:
            raise ValueError(f"companion overlay cannot reference itself: {target.value}")
        if target.value in seen:
            raise ValueError(f"duplicate companion overlay: {target.value}")
        seen.add(target.value)
        digest = str(value["payload_sha256"])
        if _SHA256.fullmatch(digest) is None:
            raise ValueError(f"invalid companion payload SHA-256: {target.value}")
        load_address = int(value["load_address"])
        size = int(value["size"])
        if load_address % 4 or not 0x80000000 <= load_address < 0x80200000:
            raise ValueError(f"invalid companion load address: {target.value}")
        if size <= 0 or load_address + size > 0x80200000:
            raise ValueError(f"invalid companion payload size: {target.value}")
        calls = value.get("static_calls", [])
        if not isinstance(calls, list) or not calls:
            raise ValueError(f"missing companion static calls: {target.value}")
        static_calls: list[CompanionStaticCall] = []
        seen_calls: set[tuple[int, int]] = set()
        for call in calls:
            if not isinstance(call, dict):
                raise ValueError(f"invalid companion static call: {target.value}")
            caller_address = int(call["caller_address"])
            target_address = int(call["target_address"])
            key = (caller_address, target_address)
            if caller_address % 4 or target_address % 4:
                raise ValueError(f"unaligned companion static call: {target.value}")
            if not load_address <= target_address < load_address + size:
                raise ValueError(f"companion call outside payload: {target.value}")
            if key in seen_calls:
                raise ValueError(f"duplicate companion static call: {target.value}")
            seen_calls.add(key)
            static_calls.append(CompanionStaticCall(*key))
        evidence = str(value["evidence"]).strip()
        if not evidence:
            raise ValueError(f"missing companion evidence: {target.value}")
        abi_raw = value.get("abi")
        abi = None
        if abi_raw is not None:
            if not isinstance(abi_raw, dict):
                raise ValueError(f"invalid companion ABI: {target.value}")
            target_address = int(abi_raw.get("target_address", 0))
            prototype = str(abi_raw.get("prototype", "")).strip()
            abi_evidence = str(abi_raw.get("evidence", "")).strip()
            call_targets = {call.target_address for call in static_calls}
            if target_address not in call_targets or not prototype or not abi_evidence:
                raise ValueError(f"missing companion ABI evidence: {target.value}")
            abi = CompanionAbi(target_address, prototype, abi_evidence)
        companions.append(
            CompanionOverlay(
                target=target,
                disc_id=disc_id,
                payload_sha256=digest,
                load_address=load_address,
                size=size,
                static_calls=tuple(static_calls),
                evidence=evidence,
                abi=abi,
            )
        )
    return tuple(companions)


def _parse_path_claims(raw: dict[str, Any], key: str) -> tuple[str, ...]:
    """Parse canonical repo-relative path claims.

    Rejects absolute paths, escape/alias spellings (``.``/``..`` components,
    duplicate slashes, trailing slashes, backslashes), and characters that
    corrupt the CMake handoff (``|``, ``;``, newline, carriage return).  Only
    the exact lexical-canonical spelling is accepted.
    """

    values = raw.get(key)
    if values is None:
        return ()
    if not isinstance(values, list) or not all(isinstance(v, str) for v in values):
        raise ValueError(f"{key} must be an array of strings")
    seen: set[str] = set()
    for value in values:
        if not value:
            raise ValueError(f"invalid {key} path: {value!r}")
        path = Path(value)
        if (
            path.is_absolute()
            or not value.isascii()
            or any(part in {"", ".", ".."} for part in path.parts)
            or any(char in value for char in "|;\n\r\\")
            or not re.fullmatch(r"[A-Za-z0-9_./-]+", value)
        ):
            raise ValueError(f"invalid {key} path: {value!r}")
        canonical = str(Path(*path.parts))
        if canonical != value:
            raise ValueError(
                f"non-canonical {key} path: {value!r} (use {canonical!r})"
            )
        if value in seen:
            raise ValueError(f"duplicate {key} path: {value}")
        seen.add(value)
    return tuple(values)


def _parse_single_path_claim(raw: dict[str, Any], key: str) -> str:
    """Parse one canonical repo-relative path claim (e.g. ``psyq_source``).

    Reuses the ``_parse_path_claims`` validation rules for a scalar string:
    empty or missing claims stay empty; anything else must be one canonical
    repo-relative path.
    """

    value = raw.get(key)
    if value is None or value == "":
        return ""
    if not isinstance(value, str):
        raise ValueError(f"{key} must be a string")
    parsed = _parse_path_claims({key: [value]}, key)
    if parsed[0] != value:
        raise ValueError(f"invalid {key} path: {value!r}")
    return value


def _validate_companions(manifests: dict[str, TargetManifest]) -> None:
    for manifest in manifests.values():
        for companion in manifest.companions:
            target = manifests.get(companion.target.value)
            if target is None:
                raise ValueError(f"unknown companion overlay: {companion.target.value}")
            if (
                target.kind != "emi"
                or target.disc_id != companion.disc_id
                or target.load_address != companion.load_address
            ):
                raise ValueError(
                    f"companion overlay identity mismatch: {companion.target.value}"
                )


def _validate_claim_overlap(manifests: dict[str, TargetManifest]) -> None:
    """Reject one path claimed twice by the same target (sources ∩ support_sources)."""

    for manifest in manifests.values():
        overlap = set(manifest.sources) & set(manifest.support_sources)
        if overlap:
            raise ValueError(
                f"claimed path in both sources and support_sources for "
                f"{manifest.id.value}: {sorted(overlap)[0]}"
            )



def _validate_claim_files(root: Path, manifests: dict[str, TargetManifest]) -> None:
    """Require every claimed path to exist with its expected file kind and stay
    inside the repository (no symlink escape)."""

    kinds = {
        "sources": {".c", ".s", ".S"},
        "support_sources": {".c", ".s", ".S"},
        "headers": {".h"},
    }
    root_resolved = root.resolve()
    for manifest in manifests.values():
        for key in ("sources", "support_sources", "headers"):
            for claimed in getattr(manifest, key):
                path = root / claimed
                if not path.is_file():
                    raise ValueError(
                        f"claimed {key} file missing for {manifest.id.value}: {claimed}"
                    )
                if path.suffix not in kinds[key]:
                    raise ValueError(
                        f"claimed {key} path has unexpected kind for "
                        f"{manifest.id.value}: {claimed} "
                        f"(expected one of {sorted(kinds[key])})"
                    )
                if not path.resolve().is_relative_to(root_resolved):
                    raise ValueError(
                        f"claimed {key} path escapes repository for "
                        f"{manifest.id.value}: {claimed}"
                    )


def _validate_claimed_paths(manifests: dict[str, TargetManifest]) -> None:
    """Reject one path claimed by two targets (ownership is target-qualified)."""

    claims: dict[str, str] = {}
    for manifest in manifests.values():
        for claimed in (
            manifest.sources + manifest.support_sources + manifest.headers
        ):
            owner = claims.get(claimed)
            if owner is not None and owner != manifest.id.value:
                raise ValueError(
                    f"claimed source path {claimed} is owned by both "
                    f"{owner} and {manifest.id.value}"
                )
            claims[claimed] = manifest.id.value


def load_target_manifests(root: Path) -> dict[str, TargetManifest]:
    directory = root / "config" / "targets"
    manifests: dict[str, TargetManifest] = {}
    if not directory.is_dir():
        return manifests
    for path in sorted(directory.rglob("*.toml")):
        raw = _load_toml(path)
        if raw.get("schema") != "harness.target/v2":
            raise ValueError(
                f"unsupported target manifest schema in {path}: {raw.get('schema')!r}"
            )
        target_id = normalize_target_id(str(raw["id"]))
        psyq = raw.get("psyq", {})
        libraries = {
            name: tuple(str(member) for member in value.get("members", []))
            for name, value in psyq.get("libraries", {}).items()
        }
        library_confidence = {
            name: str(value.get("confidence", ""))
            for name, value in psyq.get("libraries", {}).items()
            if value.get("confidence") is not None
        }
        library_evidence = {
            name: tuple(str(item) for item in value.get("evidence", []))
            for name, value in psyq.get("libraries", {}).items()
            if value.get("evidence")
        }
        placements: dict[int, list[SectionPlacement]] = {}
        seen_placements: set[tuple[int, str]] = set()
        for value in raw.get("matching", {}).get("section_placements", []):
            placement = SectionPlacement(
                function=int(value["function"]),
                section=str(value["section"]),
                address=int(value["address"]),
                size=int(value["size"]),
            )
            key = (placement.function, placement.section)
            if not re.fullmatch(r"\.[A-Za-z0-9_.]+", placement.section):
                raise ValueError(f"invalid matching section name: {placement.section}")
            if placement.function % 4 or placement.address % 4:
                raise ValueError(
                    "matching function and section addresses must be aligned"
                )
            if placement.size <= 0:
                raise ValueError("matching section placement size must be positive")
            if key in seen_placements:
                raise ValueError(
                    f"duplicate matching section placement: {placement.function:#x} "
                    f"{placement.section}"
                )
            seen_placements.add(key)
            placements.setdefault(placement.function, []).append(placement)
        manifest = TargetManifest(
            id=target_id,
            disc_id=str(raw.get("disc_id", target_id.shipped)),
            kind=str(raw["kind"]),
            source_dir=str(raw["source_dir"]),
            binary=str(raw["binary"]),
            splat=str(raw["splat"]),
            load_address=int(raw.get("load_address", 0)),
            psyq_space=str(psyq.get("space", "slus")),
            libraries=libraries,
            library_confidence=library_confidence,
            library_evidence=library_evidence,
            sources=_parse_path_claims(raw, "sources"),
            support_sources=_parse_path_claims(raw, "support_sources"),
            headers=_parse_path_claims(raw, "headers"),
            psyq_source=_parse_single_path_claim(raw, "psyq_source"),
            section_placements={
                function: tuple(values) for function, values in placements.items()
            },
            companions=_parse_companions(raw, target_id),
        )
        binary_path = root / manifest.binary
        if binary_path.is_file():
            target_end = manifest.load_address + binary_path.stat().st_size
            for values in manifest.section_placements.values():
                for placement in values:
                    if not (
                        manifest.load_address <= placement.function < target_end
                        and manifest.load_address <= placement.address
                        and placement.address + placement.size <= target_end
                    ):
                        raise ValueError(
                            f"matching section placement outside target {manifest.id.value}: "
                            f"{placement.section} at {placement.address:#x}"
                        )
        if manifest.id.value in manifests:
            raise ValueError(f"duplicate target manifest: {manifest.id.value}")
        manifests[manifest.id.value] = manifest
    _validate_companions(manifests)
    _validate_claim_overlap(manifests)
    _validate_claimed_paths(manifests)
    _validate_claim_files(root, manifests)
    for manifest in manifests.values():
        if (
            manifest.has_explicit_sources
            and manifest.psyq_source
            and manifest.psyq_source
            not in manifest.sources + manifest.support_sources
        ):
            raise ValueError(
                f"{manifest.id.value}: psyq_source must be explicitly claimed "
                "in sources or support_sources"
            )
    return manifests
