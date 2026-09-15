"""Validation-safe process-local target-manifest cache."""

from __future__ import annotations

import hashlib
import os
import stat
from dataclasses import dataclass, replace
from pathlib import Path
from types import MappingProxyType
from typing import Any, Callable, Mapping

from harness.common.deadlines import check_deadline
from harness.common.directory import open_parent_fd
from harness.common.inputs import InputBoundaryError, read_input, relative
from harness.common.links import read_linked_input

TargetManifest = Any

ManifestFingerprint = tuple[tuple[str, str], ...]
ValidationFingerprint = tuple[tuple[str, int, str, str], ...]


@dataclass(frozen=True)
class ClaimFiles:
    """One canonical metadata pass over all manifest-claimed files."""

    fingerprint: ValidationFingerprint
    canonical: Mapping[str, Path]
    sizes: Mapping[str, int]


_CACHE: dict[tuple[Path, ManifestFingerprint], dict[str, TargetManifest]] = {}


def collect_manifest_paths(root: Path) -> list[str]:
    """Discover the complete repository-relative TOML set consumed by the loader."""
    check_deadline()
    paths = []
    pending = ["config/targets"]
    seen = set()
    entries_seen = 0
    while pending:
        check_deadline()
        name = pending.pop()
        try:
            parent, leaf = open_parent_fd(root, name)
            try:
                descriptor = os.open(
                    leaf,
                    os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC,
                    dir_fd=parent,
                )
            finally:
                os.close(parent)
        except FileNotFoundError:
            if name == "config/targets":
                check_deadline()
                return []
            raise
        try:
            status = os.fstat(descriptor)
            identity = (status.st_dev, status.st_ino)
            if identity in seen or len(seen) >= 16384:
                raise ValueError("manifest directory inventory is aliased or oversized")
            seen.add(identity)
            with os.scandir(descriptor) as entries:
                for entry in entries:
                    check_deadline()
                    entries_seen += 1
                    if entries_seen > 65536:
                        raise ValueError("manifest tree entry count exceeds its bound")
                    relative = f"{name}/{entry.name}"
                    status = entry.stat(follow_symlinks=False)
                    if entry.name.endswith(".toml"):
                        if not (
                            stat.S_ISREG(status.st_mode) or stat.S_ISLNK(status.st_mode)
                        ):
                            raise ValueError(
                                f"manifest is not a regular file: {relative}"
                            )
                        paths.append(relative)
                    elif stat.S_ISDIR(status.st_mode):
                        pending.append(relative)
        finally:
            os.close(descriptor)
    paths.sort()
    check_deadline()
    return paths


def read_manifest_inputs(
    root: Path, *, read_manifest: Callable[[str], bytes] | None = None
) -> tuple[ManifestFingerprint, dict[Path, bytes]]:
    """Read each manifest once and return its content-keyed generation."""
    contents = {}
    for name in collect_manifest_paths(root):
        check_deadline()
        path = root / name
        content = (
            read_linked_input(root, name)
            if read_manifest is None
            else read_manifest(name)
        )
        if not isinstance(content, bytes):
            raise ValueError(f"manifest reader must return immutable bytes: {name}")
        contents[path] = content
    fingerprint = tuple(
        (path.as_posix(), hashlib.sha256(content).hexdigest())
        for path, content in contents.items()
    )
    check_deadline()
    return fingerprint, contents


def clone_manifest(manifest: TargetManifest) -> TargetManifest:
    return replace(
        manifest,
        libraries=manifest.libraries.copy(),
        library_confidence=manifest.library_confidence.copy(),
        library_evidence=manifest.library_evidence.copy(),
        section_placements=manifest.section_placements.copy(),
    )


def collect_claim_paths(manifests: Mapping[str, TargetManifest]) -> list[str]:
    """Return every binary, source and header consulted by claim validation."""
    paths = {manifest.binary for manifest in manifests.values() if manifest.binary}
    for manifest in manifests.values():
        paths.update(manifest.sources)
        paths.update(manifest.support_sources)
        paths.update(manifest.headers)
    return sorted(relative(path) for path in paths)


def _observe_claim_parent(path: Path) -> tuple | None:
    check_deadline()
    try:
        status = path.stat(follow_symlinks=False)
    except (FileNotFoundError, NotADirectoryError):
        return None
    return (
        status.st_dev,
        status.st_ino,
        status.st_mode,
        status.st_mtime_ns,
        status.st_ctime_ns,
    )


def collect_claim_files(
    root: Path,
    manifests: dict[str, TargetManifest],
    *,
    read_claim: Callable[[str], bytes | None] | None = None,
) -> ClaimFiles:
    """Bind claim validation to one exact byte sample per consulted path."""
    rows = []
    canonical = {}
    sizes = {}
    parents = {}
    aliases = {}
    for relative_path in collect_claim_paths(manifests):
        check_deadline()
        path = root / relative_path
        resolved = path
        if read_claim is not None:
            content = read_claim(relative_path)
        else:
            observation = parents.get(path.parent)
            if observation is None:
                parent = path.parent.resolve(strict=False)
                parents[path.parent] = (parent, _observe_claim_parent(parent))
            else:
                parent = observation[0]
            if not parent.is_relative_to(root):
                raise InputBoundaryError(relative_path)
            resolved = parent / path.name
            if resolved.is_symlink():
                resolved = resolved.resolve(strict=False)
                aliases[path] = resolved
            if not resolved.is_relative_to(root):
                raise InputBoundaryError(relative_path)
            if resolved.parent != parent and resolved.parent not in parents:
                parents[resolved.parent] = (
                    resolved.parent,
                    _observe_claim_parent(resolved.parent),
                )
            _, content = read_input(resolved)
        if content is not None and not isinstance(content, bytes):
            raise ValueError(
                f"claim reader must return immutable bytes or None: {relative_path}"
            )
        if content is not None:
            rows.append(
                (
                    relative_path,
                    len(content),
                    hashlib.sha256(content).hexdigest(),
                    resolved.as_posix(),
                )
            )
            canonical[relative_path] = resolved
            sizes[relative_path] = len(content)
        else:
            rows.append((relative_path, -1, "", ""))
            sizes[relative_path] = -1
    for parent, (expected, observed) in parents.items():
        message = f"manifest claim directory changed during validation: {parent}"
        try:
            current = parent.resolve(strict=False)
        except OSError as error:
            raise ValueError(message) from error
        if current != expected or _observe_claim_parent(current) != observed:
            raise ValueError(message)
    for path, expected in aliases.items():
        if path.resolve(strict=False) != expected:
            raise ValueError(f"manifest claim alias changed during validation: {path}")
    check_deadline()
    return ClaimFiles(tuple(rows), MappingProxyType(canonical), MappingProxyType(sizes))


def get_manifests(
    root: Path, fingerprint: ManifestFingerprint
) -> dict[str, TargetManifest] | None:
    cached = _CACHE.get((root, fingerprint))
    if cached is None:
        return None
    return {key: clone_manifest(value) for key, value in cached.items()}


def store_manifests(
    root: Path,
    fingerprint: ManifestFingerprint,
    manifests: dict[str, TargetManifest],
) -> None:
    _CACHE.clear()
    _CACHE[(root, fingerprint)] = {
        key: clone_manifest(value) for key, value in manifests.items()
    }
