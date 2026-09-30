"""Select registered domain adapters and discover their input patch folders."""

from __future__ import annotations

from ..runtime.patches import create_target
from .catalog import NAME

TARGETS = {"pcsx-redux": create_target}


def select_names(layout, names: list[str] | None, *, registered: bool = True):
    folder = layout.root / "inputs/patches"
    if folder.resolve() != folder.absolute() or not folder.is_dir():
        raise ValueError("inputs/patches must be a regular directory")
    selected = (
        names
        if names
        else sorted(path.name for path in folder.iterdir() if path.is_dir())
    )
    if (not selected and registered) or len(selected) != len(set(selected)):
        raise ValueError("no patch targets or duplicate target selection")
    if any(not NAME.fullmatch(name) or name in {".", ".."} for name in selected):
        raise ValueError("invalid patch target folder name")
    unknown = set(selected) - TARGETS.keys()
    if unknown and registered:
        raise ValueError(f"unsupported patch targets: {sorted(unknown)}")
    return selected


def select_targets(layout, names: list[str] | None):
    selected = select_names(layout, names)
    return [TARGETS[name](layout) for name in selected]
