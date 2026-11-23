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

from harness.build.compilation import compile_objects_parallel
from harness.build.compiler import CONFIGURATION_PATH, sanitize_identifier
from harness.common.lease import acquire_writer
from harness.common.lexicon import mask_c_text
from harness.decomp.consumers import identifier_consumers
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.ids import normalize_target_id
from harness.domain.manifests import load_target_manifests
from harness.domain.tags import count_function_metadata
from harness.domain.symbols import load_map, map_path, shared_map_path
from harness.match.asm_diff import match_target_source

from .schema import name_report, token_index

CONFIG = Path("config") / "targets"
_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")

# One identifier census per repository keeps a wave from rescanning every claimed
# source for each rename. A rename only rewrites the owning target, and a foreign
# reference refuses the rename, so another target's spelling cannot change under
# the census while this process runs.
_CENSUS: dict[Path, tuple[dict[str, list[tuple[str, str]]], dict[str, set[str]]]] = {}
_MEASURED: dict[tuple[str, int, int], dict] = {}


def _boundary(name: str) -> re.Pattern[str]:
    return re.compile(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"(?![A-Za-z0-9_])")


def _definition(name: str) -> re.Pattern[str]:
    """A function definition: a type and the name at the start of a line.

    Anchoring matters: a call inside an `if (...)` condition is followed by a
    brace too, and matching one made the transaction believe an unrelated unit
    owned the name.
    """
    return re.compile(
        r"^[A-Za-z_][A-Za-z0-9_ \t\*]*\b"
        + re.escape(name)
        + r"\s*\([^;{]*\)\s*\{",
        re.MULTILINE | re.S,
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
    manifests: dict | None = None
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
    plan = RenamePlan(root, target, old, new, manifest=manifest, manifests=manifests)

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
    if plan.source is not None:
        for owner, other in sorted(manifests.items()):
            if owner == target:
                continue
            if plan.source in manifest_source_paths(root, other):
                plan.refuse(
                    "shared-unit",
                    f"{plan.source.relative_to(root).as_posix()} is also claimed by {owner}",
                )
                break
    _collect_files(plan, root, manifest)
    _check_consumers(plan, root, target, old)
    return plan


def _unit_key(root: Path, path: Path) -> str:
    """The per-object compiler key for one unit, as the build sanitises it."""
    return sanitize_identifier(path.relative_to(root / "src").as_posix())


def _owned_source(root: Path, manifest, name: str) -> Path | None:
    """Return the claimed unit that owns this name.

    A file whose stem is the name owns it even when the unit carries no function
    definition, which is how a dispatch-table unit is named.
    """
    sources = manifest_source_paths(root, manifest)
    for path in sources:
        # A unit may keep an address tail while its map row does not.
        if path.is_file() and _stem_base(path.stem) == _stem_base(name):
            return path
    for path in sources:
        if path.is_file() and _definition(name).search(path.read_text(encoding="utf-8")):
            return path
    return None


def _collect_files(plan: RenamePlan, root: Path, manifest) -> None:
    boundary = _boundary(plan.old)
    splat = root / manifest.splat
    map_file = map_path(root, plan.target)
    claim = root / CONFIG / plan.target / "target.toml"
    config = [splat, map_file, claim]
    owned = list(manifest_source_paths(root, manifest)) + list(
        manifest_header_paths(root, manifest)
    )

    grouped = False
    if plan.source is not None and _stem_base(plan.source.stem) == _stem_base(plan.old):
        grouped = count_function_metadata(plan.source.read_text(encoding="utf-8")) > 1
        if not grouped:
            # A grouped unit keeps its filename: the name follows its first
            # function, and one rename must not churn the unit others use.
            plan.renames.append(
                (plan.source, plan.source.with_name(plan.new + ".c"))
            )

    boundary_base = _boundary(_stem_base(plan.old))

    def mentions(path: Path) -> bool:
        if not path.is_file():
            return False
        text = path.read_text(encoding="utf-8")
        return (
            bool(boundary.search(text))
            or bool(boundary_base.search(text))
            or any(
                before.relative_to(root).as_posix() in text
                for before, _after in plan.renames
            )
        )

    for path in owned + config:
        if mentions(path) and path not in plan.files:
            plan.files.append(path)

    if plan.renames:
        # Per-object flags and compiler ids are keyed by the unit's own path.
        flags = root / CONFIGURATION_PATH
        if flags.is_file() and _unit_key(root, plan.source) in flags.read_text(
            encoding="utf-8"
        ):
            plan.files.append(flags)
        return

    # A path claim that embeds the old spelling cannot follow a name whose owning
    # unit is not being renamed: that would leave a dangling claim.
    for path in config:
        if path.is_file() and re.search(
            re.escape(plan.old) + r"[^\s\"']*\.c", path.read_text(encoding="utf-8")
        ):
            plan.refuse(
                "path-claim",
                f"{path.relative_to(root).as_posix()} names a file after {plan.old} "
                "but no owned unit carries that stem",
            )


def _stem_base(stem: str) -> str:
    """A unit stem without its trailing address tail, when it has one."""
    return re.sub(r"_[0-9A-F]{8}$", "", stem)


def _census(root: Path, manifests):
    """Index which target files mention which map name, once per repository.

    Returns ``(mentions, owned)``: the mention index, and each target's own map
    names, so a duplicated spelling in another target stays a definition rather
    than becoming a consumer.
    """
    cached = _CENSUS.get(root)
    if cached is not None:
        return cached
    names: set[str] = set()
    owned: dict[str, set[str]] = {}
    files: list[tuple[str, Path]] = []
    for owner, manifest in sorted(manifests.items()):
        owned[owner] = {symbol.name for symbol in load_map(map_path(root, owner))}
        names.update(owned[owner])
        for source in (
            manifest_source_paths(root, manifest)
            + manifest_header_paths(root, manifest)
        ):
            if source.suffix in {".c", ".h"} and source.is_file():
                files.append((owner, source))
    rows: dict[str, list[tuple[str, str]]] = {}
    for owner, source in files:
        try:
            tokens = set(_IDENTIFIER.findall(mask_c_text(source.read_text(encoding="utf-8"))))
        except (OSError, UnicodeError):
            continue
        for name in names & tokens:
            rows.setdefault(name, []).append(
                (owner, source.relative_to(root).as_posix())
            )
    cached = (rows, owned)
    _CENSUS[root] = cached
    return cached


def _check_consumers(plan: RenamePlan, root: Path, target: str, old: str) -> None:
    try:
        mentions, owned = _census(
            root, plan.manifests or load_target_manifests(root)
        )
        findings = [
            (owner, source)
            for owner, source in mentions.get(old, ())
            if owner != target and old not in owned.get(owner, ())
        ]
    except (OSError, ValueError, RuntimeError) as error:
        plan.refuse("consumers", f"consumer scan failed: {error}")
        return
    for owner, source in findings:
        plan.refuse("fan-out", f"{owner} references {old} in {source}")


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
            wrote = False
            try:
                before = _measure(plan, plan.source)
                wrote = True
                try:
                    _write(plan, old, new)
                except (RuntimeError, ValueError, OSError) as error:
                    raise RuntimeError(f"write refused: {error}") from error
                # The manifest lists claimed unit paths, so the generation that
                # describes the renamed tree is the one the proof must read.
                plan.manifests = load_target_manifests(root)
                plan.manifest = plan.manifests.get(plan.target) or plan.manifest
                after = _measure(plan, _source_after(plan))
            except (RuntimeError, ValueError, OSError) as error:
                if wrote:
                    try:
                        _write(plan, new, old)
                    except (RuntimeError, ValueError, OSError):
                        pass
                plan.refuse("measure", str(error).strip()[:200])
                return _payload(root, plan, applied=False, proof=(), attempted=True)
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


def apply_batch(root: Path, requests: list[tuple[str, str, str]], *, jobs: int = 4) -> list[dict]:
    """Apply several renames with shared manifest and parallel compilation work.

    Each row keeps its own write, proof and rollback. The batch only overlaps the
    parts that do not interact: one manifest generation for the plan, one parallel
    compile of the plan sources, one reload after the writes, and one parallel
    compile of the renamed units before their proofs.
    """
    root = root.resolve()
    plans = [_resolve(root, target, old, new) for target, old, new in requests]
    ready = [plan for plan in plans if plan.passed]
    payloads: list[dict] = []
    for plan in plans:
        if not plan.passed:
            payloads.append(_payload(root, plan, applied=False, proof=(), attempted=True))
    if not ready:
        return payloads

    compile_objects_parallel(
        root, [plan.source for plan in ready if plan.source is not None], jobs=jobs
    )
    results: list[tuple[RenamePlan, dict]] = []
    try:
        with acquire_writer(root):
            for plan in ready:
                try:
                    before = _measure(plan, plan.source)
                    _write(plan, plan.old, plan.new)
                except (RuntimeError, ValueError, OSError) as error:
                    plan.refuse("write", str(error).strip()[:200])
                    payloads.append(
                        _payload(root, plan, applied=False, proof=(), attempted=True)
                    )
                    continue
                results.append((plan, before))

            if results:
                try:
                    generation = load_target_manifests(root)
                except (ValueError, OSError) as error:
                    for plan, _before in results:
                        _write(plan, plan.new, plan.old)
                        plan.refuse("manifests", str(error).strip()[:200])
                    return payloads + [
                        _payload(root, plan, applied=False, proof=(), attempted=True)
                        for plan, _before in results
                    ]
                moved = [
                    _source_after(plan)
                    for plan, _ in results
                    if _source_after(plan) is not None
                ]
                compile_objects_parallel(root, moved, jobs=jobs)

            for plan, before in results:
                plan.manifests = generation
                plan.manifest = generation.get(plan.target) or plan.manifest
                try:
                    after = _measure(plan, _source_after(plan))
                except (RuntimeError, ValueError, OSError) as error:
                    _write(plan, plan.new, plan.old)
                    plan.refuse("measure", str(error).strip()[:200])
                    payloads.append(
                        _payload(root, plan, applied=False, proof=(before,), attempted=True)
                    )
                    continue
                if before != after:
                    _write(plan, plan.new, plan.old)
                    plan.refuse("proof", "the renamed source no longer measures the same")
                    payloads.append(
                        _payload(
                            root, plan, applied=False, proof=(before, after), attempted=True
                        )
                    )
                    continue
                payloads.append(
                    _payload(root, plan, applied=True, proof=(before, after), attempted=True)
                )
    except RuntimeError as error:
        for plan in ready:
            if not any(row is plan for row, _ in results):
                continue
            plan.refuse("lease", str(error))
        payloads.extend(
            _payload(root, plan, applied=False, proof=(), attempted=True)
            for plan in ready
            if any(row is plan for row, _ in results)
        )
    return payloads


def _source_after(plan: RenamePlan) -> Path | None:
    """The unit's path after the rename: only a renamed unit moves."""
    if plan.source is None:
        return None
    for before, after in plan.renames:
        if before == plan.source:
            return after
    return plan.source


def _measure(plan: RenamePlan, source: Path | None) -> dict:
    """Measure one source; a rename must leave this result identical.

    A unit measured at the same path, size and mtime is reused: a wave that
    renames several symbols of one unit compiles that unit once.
    """
    if source is None or plan.address is None:
        return {"selector": None, "data": True}
    stat = source.stat()
    key = (source.as_posix(), stat.st_mtime_ns, stat.st_size)
    spent = _MEASURED.get(key)
    if spent is not None:
        return spent
    # The manifest lists claimed unit paths, so it must describe the tree as it
    # is now: a rename has already moved the file this measurement reads.
    manifest = plan.manifest
    payload = match_target_source(
        plan.root, manifest, plan.address, source, diagnostics=False
    )
    result = {
        "selector": f"{plan.target}@0x{plan.address:08X}",
        "exact": bool(payload.get("exact_match")),
        "byte_match": payload.get("byte_match"),
        "original_size": payload.get("original_size"),
        "current_size": payload.get("current_size"),
        "size_delta": payload.get("size_delta"),
        "status": payload.get("status"),
    }
    if len(_MEASURED) > 2048:
        _MEASURED.clear()
    _MEASURED[key] = result
    return result


def _write(plan: RenamePlan, source_name: str, target_name: str) -> None:
    """Rewrite the owned surface, then move the source file (undo reverses)."""
    forward = source_name == plan.old
    if forward:
        # Check every planned unit before touching a file, so a stale plan cannot
        # leave rewritten claims behind a rename that never happened.
        for before, after in plan.renames:
            if not before.is_file():
                raise ValueError(
                    f"planned unit is missing: {before.relative_to(plan.root).as_posix()}"
                )
            if after.exists():
                raise ValueError(
                    f"rename destination already exists: {after.relative_to(plan.root).as_posix()}"
                )
    if not forward:
        # Undo: restore the file name first so its old path carries the content.
        for before, after in plan.renames:
            if after.is_file() and not before.exists():
                after.rename(before)
    pattern = _boundary(source_name)
    # A claim or Splat path names the unit's file exactly; an address tail or a
    # neighbouring path segment would defeat a word-boundary rewrite.
    paths = [
        (
            before.relative_to(plan.root).as_posix(),
            after.relative_to(plan.root).as_posix(),
        )
        for before, after in plan.renames
    ]
    if plan.source is not None and source_name == plan.old and plan.renames:
        after = plan.source.with_name(plan.new + ".c")
        before_key = _unit_key(plan.root, plan.source)
        after_key = _unit_key(plan.root, after)
    else:
        before_key, after_key = "", ""
    for path in plan.files:
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8")
        rewritten = pattern.sub(target_name, text)
        for old_path, new_path in paths:
            rewritten = (
                rewritten.replace(old_path, new_path)
                if forward
                else rewritten.replace(new_path, old_path)
            )
        if before_key and path == plan.root / CONFIGURATION_PATH:
            rewritten = rewritten.replace(before_key, after_key)
        if rewritten != text:
            path.write_text(rewritten, encoding="utf-8")
    if forward:
        for before, after in plan.renames:
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
