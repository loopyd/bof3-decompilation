"""Immutable canonical repository layout and per-invocation file state."""

from __future__ import annotations

import hashlib
import io
from dataclasses import dataclass
from pathlib import Path
from types import MappingProxyType
from typing import Mapping

from harness.common.links import read_linked_state

from .claims import manifest_header_paths, manifest_source_paths
from .layout import ReviewedSplatLayout, parse_splat_text
from .manifests import TargetManifest, load_manifest_generation


@dataclass(frozen=True)
class RepositoryFiles:
    """Immutable contained file state built in one canonical traversal."""

    root: Path
    paths: tuple[Path, ...]
    stats: Mapping[Path, tuple[int, int]]
    contents: Mapping[Path, bytes]
    digests: Mapping[Path, str]
    canonical_paths: Mapping[Path, Path]
    absent_paths: frozenset[Path]

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

    def read_optional(self, path: Path) -> bytes | None:
        """Read an owned input or return its explicitly captured absence."""
        lexical = path if path.is_absolute() else self.root / path
        return None if lexical in self.absent_paths else self.read(path)

    def text(self, path: Path) -> str:
        """Decode captured UTF-8 with the ordinary universal-newline convention."""
        return io.StringIO(self.read(path).decode("utf-8"), newline=None).read()


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
    canonical_paths: dict[Path, Path] = {}
    stats: dict[Path, tuple[int, int]] = {}
    contents: dict[Path, bytes] = {}
    digests: dict[Path, str] = {}
    identities: dict[Path, tuple[int, ...]] = {}
    absent: set[Path] = set()

    def capture(name: str, *, missing_ok: bool = False) -> bytes | None:
        path = root / name
        if path in absent:
            if missing_ok:
                return None
            raise FileNotFoundError(f"repository input absent during capture: {name}")
        if path in canonical_paths:
            return contents[canonical_paths[path]]
        captured = read_linked_state(root, name, missing_ok=missing_ok)
        if captured is None:
            absent.add(path)
            return None
        content, metadata, resolved = captured
        identity = tuple(
            getattr(metadata, field)
            for field in (
                "st_dev",
                "st_ino",
                "st_mode",
                "st_nlink",
                "st_uid",
                "st_gid",
                "st_size",
                "st_mtime_ns",
                "st_ctime_ns",
            )
        )
        if resolved in contents and (
            contents[resolved] != content or identities[resolved] != identity
        ):
            raise ValueError(f"repository input changed between aliases: {name}")
        canonical_paths[path] = resolved
        canonical_paths[resolved] = resolved
        stats[resolved] = (metadata.st_size, metadata.st_mtime_ns)
        contents[resolved] = content
        identities[resolved] = identity
        digests[resolved] = hashlib.sha256(content).hexdigest()
        return content

    def capture_claim(name: str) -> bytes | None:
        return capture(name, missing_ok=True)

    generation = load_manifest_generation(
        root, read_manifest=capture, read_claim=capture_claim
    )
    manifests = generation.manifests
    splats = {
        target: parse_splat_text(
            io.StringIO(capture(manifest.splat).decode("utf-8"), newline=None).read(),
            manifest.load_address,
            origin=root / manifest.splat,
        )
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
                "include/base/compiler.h",
                "include/bof3/asm.h",
                "include/bof3/symbols.h",
                "include/include_asm.h",
            )
            if (root / relative).is_file()
        ),
        *templates,
    }
    optional_paths = {root / "config/targets/shared/symbols.txt"}
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
        optional_paths.add(reviewed)
        paths.add(root / "out/reverse/snapshots" / f"{target.replace('/', '--')}.json")
        paths.update(sources[target])
        paths.update(headers[target])
    for path in sorted(optional_paths):
        if capture(path.relative_to(root).as_posix(), missing_ok=True) is not None:
            paths.add(path)
    ordered_paths = tuple(sorted(paths))
    for path in ordered_paths:
        capture(path.relative_to(root).as_posix())
    selected = {path: canonical_paths[path] for path in ordered_paths}
    selected.update((resolved, resolved) for resolved in tuple(selected.values()))
    resolved_paths = set(selected.values())
    files = RepositoryFiles(
        root,
        ordered_paths,
        MappingProxyType({path: stats[path] for path in resolved_paths}),
        MappingProxyType({path: contents[path] for path in resolved_paths}),
        MappingProxyType({path: digests[path] for path in resolved_paths}),
        MappingProxyType(selected),
        frozenset(optional_paths & absent),
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
