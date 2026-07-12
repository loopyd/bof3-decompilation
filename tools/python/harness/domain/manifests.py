"""TOML-backed target and toolchain profile manifests."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
import tomllib
from typing import Any

from .ids import TargetId, normalize_target_id


@dataclass(frozen=True)
class Profile:
    id: str
    compiler: str
    compiler_flags: tuple[str, ...]
    assembler: str
    linker: str
    runner: str
    headers: str | None = None
    objects: str | None = None


@dataclass(frozen=True)
class Component:
    id: str
    kind: str


@dataclass(frozen=True)
class TargetManifest:
    id: TargetId
    disc_id: str
    kind: str
    source_dir: str
    binary: str
    splat: str
    load_address: int
    profile: str
    psyq_headers: str | None = None
    libraries: dict[str, tuple[str, ...]] = field(default_factory=dict)
    library_confidence: dict[str, str] = field(default_factory=dict)
    library_evidence: dict[str, tuple[str, ...]] = field(default_factory=dict)

    def __post_init__(self) -> None:
        if self.kind not in {"executable", "emi"}:
            raise ValueError(f"unsupported target kind: {self.kind}")


def _load_toml(path: Path) -> dict[str, Any]:
    with path.open("rb") as stream:
        return tomllib.load(stream)


def load_profiles(root: Path) -> dict[str, Profile]:
    path = root / "config" / "toolchains" / "profiles.toml"
    if not path.is_file():
        return {}
    payload = _load_toml(path)
    profiles: dict[str, Profile] = {}
    for profile_id, raw in payload.get("profiles", {}).items():
        profiles[profile_id] = Profile(
            id=profile_id,
            compiler=str(raw["compiler"]),
            compiler_flags=tuple(str(flag) for flag in raw.get("compiler_flags", [])),
            assembler=str(raw["assembler"]),
            linker=str(raw["linker"]),
            runner=str(raw["runner"]),
            headers=None if raw.get("headers") is None else str(raw["headers"]),
            objects=None if raw.get("objects") is None else str(raw["objects"]),
        )
    return profiles


def load_components(root: Path) -> dict[str, Component]:
    path = root / "config" / "toolchains" / "profiles.toml"
    if not path.is_file():
        return {}
    payload = _load_toml(path)
    return {
        component_id: Component(component_id, str(raw["kind"]))
        for component_id, raw in payload.get("components", {}).items()
    }


def load_target_manifests(root: Path) -> dict[str, TargetManifest]:
    directory = root / "config" / "targets"
    manifests: dict[str, TargetManifest] = {}
    if not directory.is_dir():
        return manifests
    for path in sorted(directory.rglob("*.toml")):
        raw = _load_toml(path)
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
        manifest = TargetManifest(
            id=target_id,
            disc_id=str(raw.get("disc_id", target_id.shipped)),
            kind=str(raw["kind"]),
            source_dir=str(raw["source_dir"]),
            binary=str(raw["binary"]),
            splat=str(raw["splat"]),
            load_address=int(raw.get("load_address", 0)),
            profile=str(raw["profile"]),
            psyq_headers=(
                None if psyq.get("headers") is None else str(psyq["headers"])
            ),
            libraries=libraries,
            library_confidence=library_confidence,
            library_evidence=library_evidence,
        )
        if manifest.id.value in manifests:
            raise ValueError(f"duplicate target manifest: {manifest.id.value}")
        manifests[manifest.id.value] = manifest
    return manifests
