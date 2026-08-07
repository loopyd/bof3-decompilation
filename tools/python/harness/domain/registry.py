"""Resolved target registry.

Every target identity, path, and mapping fact lives here.  All other
modules receive a ``ResolvedTarget`` instead of independently resolving
paths from a ``TargetManifest``.
"""

from __future__ import annotations

import hashlib
import re
from dataclasses import dataclass
from pathlib import Path

from .ids import TargetId, normalize_target_id
from .manifests import TargetManifest, load_target_manifests

# Lift metadata tags: the single parsing authority for @source/@behavior.
# Comment-syntax agnostic (`/* */` or `//`); hex is case-insensitive and the
# `0x` prefix optional (legacy forms accepted, tree writes `0x` uppercase).
SOURCE_TAG_RE = re.compile(r"@source\s+(?:0x)?([0-9A-Fa-f]{8})\b")
BEHAVIOR_TAG_RE = re.compile(r"@behavior (?:UNKNOWN: .+|[^\n]+)")


def parse_source_tag(text: str) -> int | None:
    """Return the lift file's function address from its @source tag, or None.

    A tag in the same comment block as a `@behavior` identifies the file's
    function and wins immediately. Tags attached to data declarations
    (trailing on, or directly above, an extern/#define) record a symbol's
    origin address without identifying the function and are skipped.
    """

    fallback = None
    for match in SOURCE_TAG_RE.finditer(text):
        start = text.rfind("/*", 0, match.start())
        prev_end = text.rfind("*/", 0, match.start())
        end = text.find("*/", match.end())
        block = (
            text[start : end + 2]
            if start != -1 and end != -1 and prev_end < start
            else ""
        )
        if "@behavior" in block:
            return int(match.group(1), 16)
        line_start = text.rfind("\n", 0, match.start()) + 1
        before = text[line_start : match.start()]
        if "extern" in before or before.lstrip().startswith("#define"):
            continue  # trailing tag on a data declaration
        rest = text[end + 2 :].lstrip() if end != -1 else ""
        if rest.startswith(("extern", "#define", "typedef")):
            continue  # leading tag on a data declaration
        if fallback is None:
            fallback = int(match.group(1), 16)
    return fallback


def parse_behavior_tag(text: str) -> str | None:
    """Return the @behavior tag text, or None when absent."""

    match = BEHAVIOR_TAG_RE.search(text)
    return match.group(0) if match is not None else None


# Raw address-encoding symbol names; conflicts resolve by a different name or
# a suffix, never an overlay-name prefix (`SCENA16_D_*` ban).
RAW_SYMBOL_NAME_RE = re.compile(r"^(?:func|D|T)_[0-9A-Fa-f]{8}$")
PREFIXED_RAW_NAME_RE = re.compile(r"(?:^|_)(?:func|D)_[0-9A-Fa-f]{8}\b")


def parse_declaration_source_tag(text: str, name: str) -> int | None:
    """Return the @source address attached to `name`'s declaration, or None.

    Considers declaration lines (`extern ... name ...;`, `#define name`, or
    a function declaration/definition `type name(...)`). The tag is accepted
    trailing on the declaration line or in the comment block directly above
    it (blank/comment lines only between tag and declaration; the nearest
    tag wins).
    """

    for match in re.finditer(rf"\b{re.escape(name)}\b", text):
        line_start = text.rfind("\n", 0, match.start()) + 1
        line_end = text.find("\n", match.end())
        line = text[line_start : line_end if line_end != -1 else len(text)]
        stripped = line.lstrip()
        after_name = line[match.end() - line_start :].lstrip()
        is_decl = (
            "extern" in line
            or stripped.startswith("#define")
            or after_name.startswith("(")
        )
        if not is_decl:
            continue
        trailing = SOURCE_TAG_RE.search(line, match.end() - line_start)
        if trailing is not None:
            return int(trailing.group(1), 16)
        tags: list[str] = []
        pos = line_start
        while pos > 0:
            prev_end = pos - 1
            prev_start = text.rfind("\n", 0, prev_end) + 1
            prev = text[prev_start:prev_end].strip()
            comment = (
                prev == ""
                or prev.startswith(("/*", "*", "//"))
                or prev.endswith("*/")
            )
            if not comment:
                break
            tags = SOURCE_TAG_RE.findall(prev) + tags
            pos = prev_start
        if tags:
            return int(tags[-1], 16)
    return None


@dataclass(frozen=True)
class ResolvedTarget:
    """A fully resolved target with absolute repository-relative paths."""

    id: TargetId
    manifest_path: Path
    disc_id: str
    kind: str
    source_dir: Path
    binary_path: Path
    splat_path: Path
    reviewed_replay_path: Path
    load_address: int

    @property
    def binary_end(self) -> int:
        return self.load_address + self.binary_size

    @property
    def binary_size(self) -> int:
        return self.binary_path.stat().st_size

    def function_id(self, address: int) -> str:
        return f"{self.id.value}@{address:08x}"

    def input_hash(self) -> str:
        """Return a composite hash of tracked input files."""

        pieces = [
            self.manifest_path.read_bytes() if self.manifest_path.is_file() else b"",
            self.binary_path.read_bytes() if self.binary_path.is_file() else b"",
            self.splat_path.read_bytes() if self.splat_path.is_file() else b"",
        ]
        return hashlib.sha256(
            b"\x00".join(hashlib.sha256(piece).digest() for piece in pieces)
        ).hexdigest()[:16]


def resolve_target(root: Path, value: str) -> ResolvedTarget:
    """Resolve a shipped or canonical ID to a ``ResolvedTarget``.

    Raises ``ValueError`` if the ID is invalid, ``FileNotFoundError`` if
    the manifest or binary is missing, and ``RuntimeError`` if the manifest
    is structurally inconsistent with the canonical identity.
    """

    target_id = normalize_target_id(value)
    manifests = load_target_manifests(root)
    manifest = manifests.get(target_id.value)
    if manifest is None:
        raise ValueError(f"unknown target: {value!r} (canonical: {target_id.value!r})")
    manifest_path = root / "config" / "targets" / target_id.value / "target.toml"
    if not manifest_path.is_file():
        raise FileNotFoundError(f"target manifest missing: {manifest_path}")
    _validate_manifest_identity(root, target_id, manifest, manifest_path)
    binary = root / manifest.binary
    if not binary.is_file():
        raise FileNotFoundError(f"target binary missing: {manifest.binary}")
    return ResolvedTarget(
        id=target_id,
        manifest_path=manifest_path,
        disc_id=manifest.disc_id,
        kind=manifest.kind,
        source_dir=root / manifest.source_dir,
        binary_path=binary,
        splat_path=root / manifest.splat,
        reviewed_replay_path=root
        / "config"
        / "targets"
        / manifest.id.value
        / "reviewed.rz",
        load_address=manifest.load_address,
    )


def lookup_target_manifest(root: Path, value: str) -> TargetManifest | None:
    """Return the ``TargetManifest`` for a shipped or canonical selector.

    Unlike :func:`resolve_target`, this never constructs resolved paths and
    never requires a target binary to exist.  Raises ``ValueError`` if the
    selector itself is malformed; returns ``None`` for a valid but unknown
    target.
    """

    target_id = normalize_target_id(value)
    return load_target_manifests(root).get(target_id.value)


def _validate_manifest_identity(
    root: Path, target_id: TargetId, manifest: TargetManifest, manifest_path: Path
) -> None:
    """Catch manifest/identity inconsistencies early."""

    expected_path = root / "config" / "targets" / target_id.value / "target.toml"
    if manifest_path != expected_path:
        raise RuntimeError(
            f"manifest path {manifest_path.relative_to(root)} does not match "
            f"target ID {target_id.value!r}"
        )
    if target_id.kind == "executable" and manifest.kind != "executable":
        raise RuntimeError(
            f"ID {target_id.value!r} has executable kind but manifest is {manifest.kind!r}"
        )
    if target_id.kind == "emi" and manifest.kind != "emi":
        raise RuntimeError(
            f"ID {target_id.value!r} has emi kind but manifest is {manifest.kind!r}"
        )


def resolve_all_targets(root: Path) -> dict[str, ResolvedTarget]:
    """Return every promoted target resolved from manifests."""

    manifests = load_target_manifests(root)
    result: dict[str, ResolvedTarget] = {}
    for target_id_str, manifest in sorted(manifests.items()):
        try:
            resolved = resolve_target(root, target_id_str)
        except FileNotFoundError:
            continue
        result[target_id_str] = resolved
    return result


__all__ = [
    "BEHAVIOR_TAG_RE",
    "SOURCE_TAG_RE",
    "ResolvedTarget",
    "lookup_target_manifest",
    "parse_behavior_tag",
    "parse_source_tag",
    "resolve_all_targets",
    "resolve_target",
]
