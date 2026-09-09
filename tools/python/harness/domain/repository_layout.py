"""Immutable canonical repository layout and per-invocation file state."""

from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path
from types import MappingProxyType
from typing import Mapping

from .claims import manifest_header_paths, manifest_source_paths
from .layout import ReviewedSplatLayout, parse_splat_layout
from .manifests import TargetManifest, load_target_manifests


@dataclass(frozen=True)
class RepositoryFiles:
    """Immutable contained file state built in one canonical traversal."""

    root: Path
    paths: tuple[Path, ...]
    stats: Mapping[Path, tuple[int, int]]
    contents: Mapping[Path, bytes]
    digests: Mapping[Path, str]
    canonical_paths: Mapping[Path, Path]

    def canonical(self, path: Path) -> Path:
        """Return a prevalidated repository-owned regular file."""
        lexical = path if path.is_absolute() else self.root / path
        canonical = self.canonical_paths.get(lexical)
        if canonical is None:
            raise ValueError(f"unregistered repository input: {path}")
        return canonical

    def stat(self, path: Path) -> tuple[int, int]:
        """Return precomputed size and nanosecond mtime."""
        return self.stats[self.canonical(path)]

    def read(self, path: Path) -> bytes:
        """Return precomputed repository input bytes."""
        return self.contents[self.canonical(path)]

    def digest(self, path: Path) -> str:
        """Return a precomputed repository input digest."""
        return self.digests[self.canonical(path)]


@dataclass(frozen=True)
class RepositoryLayout:
    """Parsed targets plus one invocation-local repository file map."""

    manifests: Mapping[str, TargetManifest]
    splats: Mapping[str, ReviewedSplatLayout]
    source_paths: Mapping[str, tuple[Path, ...]]
    header_paths: Mapping[str, tuple[Path, ...]]
    shared_template_paths: tuple[Path, ...]
    files: RepositoryFiles
    type_input_rows: Mapping[str, tuple[tuple[str, str, str], ...]]
    macro_input_rows: Mapping[str, tuple[tuple[str, str, str, str], ...]]
    selected_map_rows: Mapping[str, tuple[tuple[str, str], ...]]


def load_repository_layout(root: Path) -> RepositoryLayout:
    """Parse each canonical manifest/Splat and discover owned paths once."""
    root = root.resolve()
    manifests = load_target_manifests(root)
    from ._manifest_cache import validated_claim_files

    claims = validated_claim_files(root)
    splats = {
        target: parse_splat_layout(root / manifest.splat, manifest.load_address)
        for target, manifest in manifests.items()
    }
    sources = {
        target: tuple(manifest_source_paths(root, manifest))
        if manifest.has_explicit_sources
        else ()
        for target, manifest in manifests.items()
    }
    headers = {
        target: tuple(manifest_header_paths(root, manifest))
        for target, manifest in manifests.items()
    }
    shared = root / "src/shared"
    templates = tuple(sorted(shared.rglob("*.inc"))) if shared.is_dir() else ()
    paths = {
        root / "include/base/types.h",
        *(
            root / relative
            for relative in (
                "include/base/barrier.h",
                "include/bof3/asm.h",
                "include/bof3/symbols.h",
                "include/include_asm.h",
            )
            if (root / relative).is_file()
        ),
        *templates,
    }
    for target, manifest in manifests.items():
        paths.update(root / relative for relative in splats[target].symbol_map_paths)
        target_config = root / "config/targets" / target
        paths.update(
            (
                root / manifest.binary,
                root / manifest.splat,
                target_config / "target.toml",
            )
        )
        # Target and selected PsyQ maps are manifest-owned inputs. Reviewed
        # Splat composition independently owns every selected symbol map.
        paths.add(target_config / "symbols.txt")
        paths.add(root / "config/sdk" / f"psyq-{manifest.psyq_space}.txt")
        reviewed = target_config / "reviewed.rz"
        if reviewed.is_file():
            paths.add(reviewed)
        paths.add(root / "out/reverse/snapshots" / f"{target.replace('/', '--')}.json")
        paths.update(sources[target])
        paths.update(headers[target])
    ordered_paths = tuple(sorted(paths))
    canonical_paths: dict[Path, Path] = {}
    stats: dict[Path, tuple[int, int]] = {}
    contents: dict[Path, bytes] = {}
    digests: dict[Path, str] = {}
    for path in ordered_paths:
        relative = path.relative_to(root).as_posix()
        resolved = claims.canonical.get(relative) if claims is not None else None
        resolved = resolved or path.resolve(strict=True)
        try:
            resolved.relative_to(root)
        except ValueError as exc:
            raise ValueError(f"repository input escapes root: {path}") from exc
        metadata = resolved.stat()
        if not resolved.is_file():
            raise ValueError(f"repository input is not a file: {path}")
        canonical_paths[path] = resolved
        canonical_paths[resolved] = resolved
        if resolved not in contents:
            content = resolved.read_bytes()
            stats[resolved] = (metadata.st_size, metadata.st_mtime_ns)
            contents[resolved] = content
            digests[resolved] = hashlib.sha256(content).hexdigest()
    files = RepositoryFiles(
        root,
        ordered_paths,
        MappingProxyType(stats),
        MappingProxyType(contents),
        MappingProxyType(digests),
        MappingProxyType(canonical_paths),
    )
    from harness.macros.index import macro_input_rows
    from harness.types.inputs import type_input_rows

    type_rows = {
        target: tuple(
            type_input_rows(
                root,
                manifest,
                digest_file=files.digest,
                source_paths=sources[target],
                header_paths=headers[target],
                validate_paths=False,
            )
        )
        for target, manifest in manifests.items()
    }
    macro_rows = {
        target: tuple(
            macro_input_rows(
                root,
                target,
                manifest,
                digest_file=files.digest,
                source_paths=sources[target],
                header_paths=headers[target],
                template_paths=templates,
                validate_paths=False,
            )
        )
        for target, manifest in manifests.items()
    }
    selected_maps = {
        target: tuple(
            (path, files.digest(root / path))
            for path in splats[target].symbol_map_paths
        )
        for target in manifests
    }
    return RepositoryLayout(
        MappingProxyType(dict(manifests)),
        MappingProxyType(splats),
        MappingProxyType(sources),
        MappingProxyType(headers),
        templates,
        files,
        MappingProxyType(type_rows),
        MappingProxyType(macro_rows),
        MappingProxyType(selected_maps),
    )
