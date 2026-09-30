"""Validate operation scope and dispatch selected patch targets."""

from __future__ import annotations

from .operations import run_operation, select_state
from .targets import select_targets
from .validation import inspect_series


def run_patches(
    layout,
    action: str,
    targets: list[str] | None = None,
    patches: list[str] | None = None,
) -> dict:
    if patches and (not targets or len(targets) != 1):
        raise ValueError("--patch requires exactly one explicit --target")
    selected = select_targets(layout, targets)
    # Preflight every target before the first target's transaction can publish.
    for target in selected:
        series = inspect_series(target)
        select_state(series, "apply" if action == "check" else action, patches)
        if action == "apply":
            target.prepare(series.evidence)
    reports = [run_operation(target, action, patches) for target in selected]
    return {"schema": "harness.patches/v1", "action": action, "targets": reports}
