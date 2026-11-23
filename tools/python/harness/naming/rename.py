"""Plan, apply and prove one semantic rename across its owned surface.

The skill chooses both spellings. This module owns the mechanical fan-out: it
refuses a name that breaks the grammar, collides in the composed map or has a
consumer outside the owning target, rewrites filename, identifier, target-owned
callers and declarations, Splat label and ``@source``, symbol row and target
claim, then proves each renamed function against its original bytes.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

from harness.common.lease import acquire_writer
from harness.decomp.consumers import identifier_consumers
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.ids import normalize_target_id
from harness.domain.manifests import load_target_manifests
from harness.domain.symbols import load_map, map_path, shared_map_path
from harness.match.asm_diff import match_target_source

from .schema import name_report, token_index

CONFIG = Path("config") / "targets"


def _boundary(name: str) -> re.Pattern[str]:
    return re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")


def _definition(name: str) -> re.Pattern[str]:
    """A function definition: the name followed by a parameter list."""
    return re.compile(
        r"(?<![A-Za-z0-9_])" + re.escape(name) + r"\s*\([^;{]*\)\s*\{", re.S
    )


@dataclass
class RenamePlan:
    root: Path
    target: str
    old: str
    new: str
    address: int | None = None
    source: Path | None = None
    manifest: object | None = None
    files: list[Path] = field(default_factory=list)
    renames: list[tuple[Path, Path]] = field(default_factory=list)
    refusals: list[dict] = field(default_factory=list)

    @property
    def passed(self) -> bool:
        return not self.refusals

    def refuse(self, code: str, message: str) -> None:
        self.refusals.append({"code": code, "message": message})


def _resolve(root: Path, target: str, old: str, new: str) -> RenamePlan:
    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        plan = RenamePlan(root, target, old, new)
        plan.refuse("target", f"unknown target: {target}")
        return plan
    manifest = manifests[target]
    plan = RenamePlan(root, target, old, new, manifest=manifest)

    if old == new:
        plan.refuse("no-op", "the old and new spellings are equal")
        return plan
    report = name_report(new, target=target, tokens_by_target=token_index(manifests))
    if not report.passed:
        plan.refuse("schema", f"{new}: {','.join(report.violations)}")

    symbols = load_map(map_path(root, target))
    row = next((symbol for symbol in symbols if symbol.name == old), None)
    if row is None:
        plan.refuse("symbol", f"{old} is not a row in {map_path(root, target)}")
        return plan
    plan.address = row.address
    if any(
        symbol.name.casefold() == new.casefold() and symbol.address != row.address
        for symbol in symbols
    ):
        plan.refuse("collision", f"{new} already exists in this target's map")

    shared = shared_map_path(root)
    if shared.is_file() and _boundary(old).search(shared.read_text(encoding="utf-8")):
        plan.refuse(
            "shared", f"{old} is claimed by {shared.relative_to(root).as_posix()}"
        )

    plan.source = _owned_source(root, manifest, old)
    _collect_files(plan, root, manifest)
    _check_consumers(plan, root, target, old)
    return plan


def _owned_source(root: Path, manifest, name: str) -> Path | None:
    sources = manifest_source_paths(root, manifest)
    preferred = [path for path in sources if path.stem == name]
    for path in preferred or sources:
        if path.is_file() and _definition(name).search(path.read_text(encoding="utf-8")):
            return path
    return None


def _collect_files(plan: RenamePlan, root: Path, manifest) -> None:
    boundary = _boundary(plan.old)
    owned = list(manifest_source_paths(root, manifest)) + list(
        manifest_header_paths(root, manifest)
    )
    for path in owned:
        if path.is_file() and boundary.search(path.read_text(encoding="utf-8")):
            plan.files.append(path)
    if plan.source is not None and plan.source not in plan.files:
        plan.files.append(plan.source)

    splat = root / manifest.splat
    map_file = map_path(root, plan.target)
    claim = root / CONFIG / plan.target / "target.toml"
    for path in (splat, map_file, claim):
        if path.is_file() and boundary.search(path.read_text(encoding="utf-8")):
            plan.files.append(path)

    if plan.source is not None and plan.source.stem == plan.old:
        plan.renames.append((plan.source, plan.source.with_name(plan.new + ".c")))


def _check_consumers(plan: RenamePlan, root: Path, target: str, old: str) -> None:
    try:
        _, findings = identifier_consumers(root, target, {old})
    except (OSError, ValueError, RuntimeError) as error:
        plan.refuse("consumers", f"consumer scan failed: {error}")
        return
    for finding in findings:
        if finding.get("code") != "foreign_consumers":
            continue
        plan.refuse(
            "fan-out",
            f"{finding.get('target')} references {old} in {finding.get('source')}",
        )


def plan_report(root: Path, target: str, old: str, new: str) -> dict:
    """Dry run: name every surface the rename would touch."""
    root = root.resolve()
    plan = _resolve(root, target, old, new)
    return _payload(root, plan, applied=False, proof=(), attempted=False)


def apply_report(root: Path, target: str, old: str, new: str) -> dict:
    """Apply the rename and prove its measurement did not change."""
    root = root.resolve()
    plan = _resolve(root, target, old, new)
    if not plan.passed:
        return _payload(root, plan, applied=False, proof=(), attempted=True)
    try:
        with acquire_writer(root):
            before = _measure(plan, plan.source)
            _write(plan, old, new)
            after = _measure(plan, _source_after(plan))
            if before != after:
                _write(plan, new, old)
                plan.refuse("proof", "the renamed source no longer measures the same")
                return _payload(
                    root, plan, applied=False, proof=(before, after), attempted=True
                )
    except RuntimeError as error:
        plan.refuse("lease", str(error))
        return _payload(root, plan, applied=False, proof=(), attempted=True)
    return _payload(root, plan, applied=True, proof=(before, after), attempted=True)


def _source_after(plan: RenamePlan) -> Path | None:
    if plan.source is None:
        return None
    if plan.source.stem == plan.old:
        return plan.source.with_name(plan.new + ".c")
    return plan.source


def _measure(plan: RenamePlan, source: Path | None) -> dict:
    """Measure one source; a rename must leave this result identical."""
    if source is None or plan.address is None:
        return {"selector": None, "data": True}
    payload = match_target_source(
        plan.root, plan.manifest, plan.address, source, diagnostics=False
    )
    return {
        "selector": f"{plan.target}@0x{plan.address:08X}",
        "exact": bool(payload.get("exact_match")),
        "byte_match": payload.get("byte_match"),
        "original_size": payload.get("original_size"),
        "current_size": payload.get("current_size"),
        "size_delta": payload.get("size_delta"),
        "status": payload.get("status"),
    }


def _write(plan: RenamePlan, source_name: str, target_name: str) -> None:
    """Rewrite the owned surface, then move the source file (undo reverses)."""
    forward = source_name == plan.old
    if not forward:
        # Undo: restore the file name first so its old path carries the content.
        for before, after in plan.renames:
            if after.is_file() and not before.exists():
                after.rename(before)
    pattern = _boundary(source_name)
    for path in plan.files:
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8")
        rewritten = pattern.sub(target_name, text)
        if rewritten != text:
            path.write_text(rewritten, encoding="utf-8")
    if forward:
        for before, after in plan.renames:
            if before.is_file():
                before.rename(after)


def _payload(
    root: Path, plan: RenamePlan, *, applied: bool, proof, attempted: bool
) -> dict:
    return {
        "operation": "naming",
        "action": "rename",
        "scope": {"target": plan.target, "old": plan.old, "new": plan.new},
        "address": f"0x{plan.address:08X}" if plan.address is not None else None,
        "source": plan.source.relative_to(root).as_posix() if plan.source else None,
        "applied": applied,
        "edits": [
            {"file": path.relative_to(root).as_posix()} for path in plan.files
        ],
        "renames": [
            {
                "from": before.relative_to(root).as_posix(),
                "to": after.relative_to(root).as_posix(),
            }
            for before, after in plan.renames
        ],
        "proof": list(proof),
        "refusals": list(plan.refusals),
        "passed": plan.passed and (applied or not attempted),
    }
