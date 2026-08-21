"""Validated Rizin snapshot loading for the derived reverse index."""

from pathlib import Path

from ..discovery import file_sha256
from .project import prepare_target
from .snapshot import read_snapshot, snapshot_path, validate_snapshot_identity


def snapshot_for(root: Path, target: str, binary: Path, *, manifest=None):
    path = snapshot_path(root, target)
    if not path.is_file():
        raise ValueError(f"missing Rizin snapshot: {path.relative_to(root)}")
    snapshot = read_snapshot(path)
    errors = validate_snapshot_identity(snapshot)
    if errors:
        raise ValueError(
            f"invalid Rizin snapshot {path.relative_to(root)}: {'; '.join(errors)}"
        )
    if snapshot.target != target:
        raise ValueError(f"stale Rizin snapshot target: {path.relative_to(root)}")
    if snapshot.engine.get("name") != "rizin":
        raise ValueError(
            f"snapshot was not produced by Rizin: {path.relative_to(root)}"
        )
    if snapshot.inputs.get("binary_sha256") != file_sha256(binary):
        raise ValueError(f"stale Rizin snapshot bytes: {path.relative_to(root)}")
    if (
        snapshot.inputs.get("replay_sha256")
        != prepare_target(root, target, manifest=manifest).replay_sha256
    ):
        raise ValueError(f"stale Rizin snapshot recipe: {path.relative_to(root)}")
    return path, snapshot
