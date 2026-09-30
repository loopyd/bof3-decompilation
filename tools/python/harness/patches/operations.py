"""Apply or revert selected patch states with preflight and recoverable publication."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import tempfile

from ..io import write_json
from .models import Series, Target
from .validation import describe_series, inspect_series, read_sources


def select_state(
    series: Series, action: str, names: list[str] | None
) -> dict[str, bytes | None]:
    known = {patch.name for patch in series.patches}
    selected = set(names) if names else known
    if selected - known or len(selected) != len(names or selected):
        raise ValueError(f"unknown or duplicate patch selection: {names}")
    desired = dict(series.observed)
    for group, count in zip(series.groups, series.counts):
        choices = [
            n for n, index in enumerate(group) if series.patches[index].name in selected
        ]
        if not choices:
            continue
        wanted = (
            max(count, max(choices) + 1)
            if action == "apply"
            else min(count, min(choices))
        )
        affected = range(count, wanted) if action == "apply" else range(wanted, count)
        missing = [series.patches[group[n]].name for n in affected if n not in choices]
        if missing:
            raise ValueError(
                f"patch scope omits required {action} dependencies: {missing}"
            )
        version = group[wanted - 1] + 1 if wanted else 0
        for name in {name for i in group for name in series.patches[i].files}:
            desired[name] = series.versions[version][name]
    return desired


def publish_file(path: Path, data: bytes | None, mode: int) -> None:
    if data is None:
        path.unlink(missing_ok=True)
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(prefix=".patch-", dir=path.parent)
    try:
        with os.fdopen(descriptor, "wb") as output:
            os.fchmod(output.fileno(), mode)
            output.write(data)
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    finally:
        Path(temporary).unlink(missing_ok=True)


def run_operation(target: Target, action: str, names: list[str] | None = None) -> dict:
    if action not in {"apply", "check", "revert"}:
        raise ValueError(f"unsupported patch operation: {action}")
    series = inspect_series(target)
    # Validate even a read-only selection before returning a check result.
    desired = select_state(series, "apply" if action == "check" else action, names)
    report = {
        "schema": "harness.patch/v1",
        "target": target.name,
        "action": action,
        "selected": names or [patch.name for patch in series.patches],
        "identity": describe_series(series),
    }
    if action == "check":
        return report
    if action == "apply":
        target.prepare(series.evidence)
    changed = [name for name in desired if desired[name] != series.observed[name]]
    if not changed:
        report["changed"] = []
        return report
    recovery_root = target.root / "out/patches"
    if recovery_root.resolve() != recovery_root.absolute():
        raise ValueError("patch recovery directory must not use symlinks")
    recovery_root.mkdir(parents=True, exist_ok=True)
    recovery = Path(
        tempfile.mkdtemp(prefix=f"{action}-{target.name}-", dir=recovery_root)
    )
    manifest = {}
    for name in changed:
        data = series.observed[name]
        if data is not None:
            publish_file(recovery / "before" / name, data, series.modes[name])
        manifest[name] = {
            "mode": series.modes[name],
            "existed": data is not None,
            "sha256": hashlib.sha256(data).hexdigest() if data is not None else None,
        }
    write_json(
        recovery / "receipt.json", {**report, "status": "prepared", "sources": manifest}
    )
    fresh = inspect_series(target)
    if (
        fresh.observed != series.observed
        or fresh.modes != series.modes
        or fresh.patches != series.patches
        or fresh.evidence != series.evidence
    ):
        raise ValueError(
            f"patch inputs changed before publication; recovery: {recovery}"
        )
    try:
        for name in changed:
            current, _ = read_sources(target.source, {name})
            if current[name] != series.observed[name]:
                raise ValueError(f"source changed before publication: {name}")
            publish_file(target.source / name, desired[name], series.modes[name])
        after = inspect_series(target)
        if after.observed != desired:
            raise ValueError("published patch state differs from expected bytes")
        report.update(
            identity=describe_series(after), changed=changed, recovery=str(recovery)
        )
        write_json(
            recovery / "receipt.json",
            {**report, "status": "passed", "sources": manifest},
        )
        return report
    except Exception as error:
        conflicts = []
        for name in reversed(changed):
            try:
                current, _ = read_sources(target.source, {name})
                if current[name] == desired[name]:
                    publish_file(
                        target.source / name, series.observed[name], series.modes[name]
                    )
                elif current[name] != series.observed[name]:
                    conflicts.append(name)
            except (OSError, ValueError):
                conflicts.append(name)
        write_json(
            recovery / "receipt.json",
            {
                **report,
                "status": "failed",
                "sources": manifest,
                "error": str(error),
                "rollback_conflicts": conflicts,
            },
        )
        raise ValueError(
            f"patch publication failed; recovery: {recovery}; preserved conflicts: {conflicts}"
        ) from error
