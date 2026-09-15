"""Fail-closed source ownership and lifecycle facts for naming audits."""

from __future__ import annotations

from collections.abc import Iterable, Mapping
from pathlib import Path

from harness.domain.claims import NoSourceClaims
from harness.domain.functions import collect_lift_metadata
from harness.domain.manifests import TargetManifest
from harness.domain.registry import resolve_target
from harness.domain.sources import SourceAddressCollision
from harness.domain.tags import parse_behavior_tag, parse_progress_tags

Progress = tuple[str, float | None, str] | ValueError | None


def read_source_metadata(paths: Iterable[Path]) -> dict[int, tuple[Path, Progress]]:
    """Read each claimed C input once, rejecting ambiguous or unreadable ownership."""
    metadata: dict[int, tuple[Path, Progress]] = {}
    for source in sorted(set(paths)):
        if source.suffix != ".c":
            continue
        try:
            records = collect_lift_metadata(source.read_text(encoding="utf-8"))
        except (OSError, ValueError) as error:
            raise ValueError(
                f"cannot inspect source metadata: {source}: {error}"
            ) from error
        for address, text in records.items():
            if parse_behavior_tag(text) is None:
                continue
            if address in metadata:
                raise SourceAddressCollision(
                    f"duplicate source owner for 0x{address:08X}: "
                    f"{metadata[address][0]} and {source}"
                )
            try:
                progress: Progress = parse_progress_tags(text)
            except ValueError as error:
                progress = error
            metadata[address] = (source, progress)
    return metadata


def describe_metadata(
    metadata: Mapping[int, tuple[Path, Progress]], address: int
) -> tuple[bool, str, bool]:
    """Return metadata validity, diagnostic and partial state for one address."""
    source, progress = metadata.get(address, (None, None))
    if source is None:
        return True, "no claimed source", False
    if isinstance(progress, ValueError):
        return False, f"{source}: {progress}", False
    return True, "metadata canonical", progress is not None and progress[0] == "partial"


def inspect_metadata(
    root: Path, manifest: TargetManifest, address: int
) -> tuple[bool, str, bool]:
    """Distinguish absent function claims from failed target/source inspection."""
    try:
        resolved = resolve_target(root, manifest.id.value)
        metadata = read_source_metadata(resolved.source_paths)
    except NoSourceClaims:
        return describe_metadata({}, address)
    except (OSError, ValueError, RuntimeError) as error:
        return False, f"source metadata unavailable: {error}", False
    return describe_metadata(metadata, address)
