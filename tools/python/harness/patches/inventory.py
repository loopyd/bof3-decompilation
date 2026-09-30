"""List patch inputs without a checkout, optionally inspecting live application state."""

from __future__ import annotations

import hashlib

from .catalog import read_patch_inputs
from . import targets as registry
from .validation import describe_series, inspect_series


def list_patches(
    layout,
    targets: list[str] | None = None,
    patches: list[str] | None = None,
    *,
    status: bool = False,
) -> dict:
    if patches and (not targets or len(targets) != 1):
        raise ValueError("--patch requires exactly one explicit --target")
    rows, valid = [], True
    for name in registry.select_names(layout, targets, registered=False):
        inputs = read_patch_inputs(
            layout.root / "inputs/patches" / name, allow_empty=True
        )
        selected = set(patches) if patches else {path.name for path, _ in inputs}
        if selected - {path.name for path, _ in inputs} or len(selected) != len(
            patches or selected
        ):
            raise ValueError(f"unknown or duplicate patch selection: {patches}")
        files = [
            {
                "name": path.name,
                "path": str(path.relative_to(layout.root)),
                "sha256": hashlib.sha256(data).hexdigest(),
                "bytes": len(data),
            }
            for path, data in inputs
            if path.name in selected
        ]
        row = {"target": name, "supported": name in registry.TARGETS, "patches": files}
        if status:
            try:
                if name not in registry.TARGETS:
                    raise ValueError(f"unsupported patch target: {name}")
                identity = describe_series(
                    inspect_series(registry.TARGETS[name](layout))
                )
                states = {item["name"]: item for item in identity["patches"]}
                for item in files:
                    state = states.get(item["name"])
                    if state is None or state["sha256"] != item["sha256"]:
                        raise ValueError("patch input changed during listing")
                    item["status"] = "applied" if state["applied"] else "pristine"
            except (OSError, ValueError) as error:
                valid = False
                row["error"] = str(error)
                for item in files:
                    item["status"] = "unresolved"
        rows.append(row)
    return {
        "schema": "harness.patches/v1",
        "action": "list",
        "status_requested": status,
        "valid": valid,
        "targets": rows,
    }
