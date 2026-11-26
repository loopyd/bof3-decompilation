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

import yaml

from harness.build.compilation import compile_objects_parallel
from harness.build.compiler import CONFIGURATION_PATH, sanitize_identifier
from harness.common.lease import acquire_writer
from harness.common.lexicon import mask_c_text
from harness.decomp.consumers import identifier_consumers
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.ids import normalize_target_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.tags import count_function_metadata
from harness.domain.symbols import load_map, map_path, shared_map_path
from harness.match._asm_resolve import extract_original_bytes
from harness.match.asm_diff import match_target_source

from .schema import name_report, token_index

CONFIG = Path("config") / "targets"
_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")

# One identifier census per repository keeps a wave from rescanning every claimed
# source for each rename. A rename only rewrites the owning target, and a foreign
# reference refuses the rename, so another target's spelling cannot change under
# the census while this process runs.
_CENSUS: dict[Path, tuple[dict[str, list[tuple[str, str]]], dict[str, set[str]]]] = {}
_NAMES: dict[Path, dict[str, list[tuple[str, int, frozenset[Path]]]]] = {}
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
    # Targets that will carry this same spelling. A name shared by a group is
    # legal only when every member's reviewed bytes are identical.
    group: frozenset[str] = frozenset()
    shared_with: list[dict] = field(default_factory=list)
    # Owner -> sources outside this target that spell the old name. A rename writes
    # them too, so one spelling holds across every unit that calls it.
    foreign: dict[str, list[str]] = field(default_factory=dict)
    # Owner -> address that owner spells but does not map. The link resolves the
    # spelling in that owner's own map, so the transaction adds the row.
    inserts: dict[str, int] = field(default_factory=dict)

    @property
    def passed(self) -> bool:
        return not self.refusals

    def refuse(self, code: str, message: str) -> None:
        self.refusals.append({"code": code, "message": message})


def _resolve(
    root: Path,
    target: str,
    old: str,
    new: str,
    *,
    group: frozenset[str] = frozenset(),
    allow_noop: bool = False,
) -> RenamePlan:
    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        plan = RenamePlan(root, target, old, new)
        plan.refuse("target", f"unknown target: {target}")
        return plan
    manifest = manifests[target]
    plan = RenamePlan(
        root, target, old, new, manifest=manifest, manifests=manifests, group=group
    )

    if old == new:
        if not allow_noop:
            plan.refuse("no-op", "the old and new spellings are equal")
            return plan
        # A group anchor already carries the spelling; it owes no write, but its
        # bytes still take part in the equality check.
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

    plan.source = _owned_source(root, manifest, old, row.address)
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
    _record_cross_target_name(plan, root, target)
    if plan.passed:
        _retarget_twin(plan, root)
    return plan


def _unit_key(root: Path, path: Path) -> str:
    """The per-object compiler key for one unit, as the build sanitises it."""
    return sanitize_identifier(path.relative_to(root / "src").as_posix())


_SOURCE_ADDRESS = re.compile(r"@source\s+(0x[0-9A-Fa-f]{8})")
_ANY_DEFINITION = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*\b[A-Za-z_][A-Za-z0-9_]*\s*\([^;{]*\)\s*\{",
    re.MULTILINE,
)


def _owned_source(root: Path, manifest, name: str, address: int | None = None) -> Path | None:
    """Return the claimed unit that owns this name.

    A file whose stem is the name owns it even when the unit carries no function
    definition, which is how a dispatch-table unit is named.
    """
    sources = manifest_source_paths(root, manifest)
    live = [path for path in sources if path.is_file()]
    for path in live:
        if path.stem == name:
            return path
    # A unit may keep an address tail while its map row does not, but a base
    # match is only an owner when exactly one unit carries that base: two
    # neighbours differing only in their address tail are different units.
    base = _stem_base(name)
    matches = []
    for path in live:
        if _stem_base(path.stem) != base:
            continue
        # The file's own metadata address decides: two symbols of one target can
        # share a stem base and still be different units.
        found = _SOURCE_ADDRESS.search(path.read_text(encoding="utf-8"))
        if address is not None and (
            found is None or int(found.group(1), 16) != address
        ):
            continue
        matches.append(path)
    if len(matches) == 1:
        return matches[0]
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
    if plan.source is not None and _carries_spelling(plan.source.stem, plan.old):
        text = plan.source.read_text(encoding="utf-8")
        # One metadata block can still carry several definitions: a dispatch
        # table unit names many handlers. Any such unit keeps its filename.
        grouped = (
            count_function_metadata(text) > 1
            or len(_ANY_DEFINITION.findall(mask_c_text(text))) > 1
        )
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


TAIL = re.compile(r"(?:Bmagic|Bboss|World\d\d|Plchar|Scenario|Scena|Emi|Batl|Commpo)\w*\d+$")

def _carries_spelling(stem: str, old: str) -> bool:
    """True when a unit's file name carries the map spelling it renames.

    A unit may keep a discriminator (`_battle03`) or the machine tail it was harvested
    with (`BmagicMagic00100_801EEF34`); both still name the same row.
    """
    base = _stem_base(stem)
    target = _stem_base(old)
    if base == target or stem.startswith(target + "_"):
        return True
    if not base.startswith(target):
        return False
    return bool(TAIL.match(base[len(target):]))


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


def _name_index(root: Path, manifests):
    """Index every map name with its owner, address and source directories.

    The symbol matrix rejects one name for different reviewed bytes in the same
    source parent, so a rename must see the other targets' spellings, not just its
    own map.
    """
    cached = _NAMES.get(root)
    if cached is not None:
        return cached
    rows: dict[str, list[tuple[str, int, frozenset[Path]]]] = {}
    for owner, manifest in sorted(manifests.items()):
        directories = frozenset(
            path.parent for path in manifest_source_paths(root, manifest)
        )
        for symbol in load_map(map_path(root, owner)):
            rows.setdefault(symbol.name.casefold(), []).append(
                (owner, symbol.address, directories)
            )
    _NAMES[root] = rows
    return rows


def _remember(root: Path, plan: RenamePlan) -> None:
    """Keep the name index true after this process has renamed a row."""
    index = _NAMES.get(root)
    if index is None or plan.address is None:
        return
    for name, rows in list(index.items()):
        kept = [row for row in rows if row[0] != plan.target or row[1] != plan.address]
        if kept:
            index[name] = kept
        else:
            del index[name]
    index.setdefault(plan.new.casefold(), []).append(
        (plan.target, plan.address, _plan_dirs(root, plan))
    )


def _plan_dirs(root: Path, plan: RenamePlan) -> frozenset[Path]:
    return frozenset(
        path.parent
        for path in manifest_source_paths(root, plan.manifest)
        if path.suffix == ".c"
    )


def _bytes_at(root: Path, manifests, target: str, address: int) -> bytes | None:
    """Original bytes of one row in one target, or None when it has no code range."""
    manifest = manifests.get(target)
    if manifest is None:
        return None
    try:
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    except (OSError, ValueError, yaml.YAMLError):
        # A layout this reader cannot parse is missing evidence, not a refusal.
        return None
    boundary = layout.find_boundary_at(address)
    if boundary is None or boundary.virtual_end is None:
        return None
    try:
        return extract_original_bytes(
            root / manifest.binary,
            address=address,
            size=boundary.virtual_end - boundary.virtual_start,
            load_address=manifest.load_address,
        )
    except (OSError, ValueError):
        return None


def _record_cross_target_name(plan: RenamePlan, root: Path, target: str) -> None:
    """Record a spelling another target of the same parent already carries.

    One source parent holds the same roles in each of its overlays, so a spelling may
    name that role in several targets even when their bytes differ: the difference is
    usually a resource address the overlay owns. The record keeps the pairing so a
    later review sees which rows now share a spelling and how their bodies compare.
    """
    if plan.address is None:
        return
    ours = _plan_dirs(root, plan)
    for owner, address, directories in _name_index(
        root, load_manifests_for(plan)
    ).get(plan.new.casefold(), ()):
        if owner == target or address == plan.address or not (ours & directories):
            continue
        if owner in plan.group:
            # A group member is about to carry this spelling; the byte check below
            # decides whether sharing it is legal.
            continue
        manifests = load_manifests_for(plan)
        ours_bytes = _bytes_at(root, manifests, target, plan.address)
        theirs = _bytes_at(root, manifests, owner, address)
        plan.shared_with.append(
            {
                "target": owner,
                "address": f"0x{address:08X}",
                "bytes": "identical" if ours_bytes is not None and ours_bytes == theirs else "different",
            }
        )


def _reviewed_bytes(root: Path, plan: RenamePlan) -> bytes | None:
    """The original bytes of one planned row, or None when it has no code range."""
    if plan.address is None or plan.manifest is None:
        return None
    try:
        layout = parse_splat_layout(
            root / plan.manifest.splat, plan.manifest.load_address
        )
    except (OSError, ValueError, yaml.YAMLError):
        return None
    boundary = layout.find_boundary_at(plan.address)
    if boundary is None or boundary.virtual_end is None:
        return None
    try:
        return extract_original_bytes(
            root / plan.manifest.binary,
            address=plan.address,
            size=boundary.virtual_end - boundary.virtual_start,
            load_address=plan.manifest.load_address,
        )
    except (OSError, ValueError):
        return None


def _record_group_bytes(plans: list[RenamePlan], root: Path) -> None:
    """Record how each member's reviewed bytes compare with the group's first body."""
    code = [(plan, _reviewed_bytes(root, plan)) for plan in plans]
    shared = [(plan, body) for plan, body in code if body is not None]
    if len(shared) < 2:
        return
    reference_plan, reference = shared[0]
    for plan, body in shared[1:]:
        same = body == reference
        plan.shared_with.append(
            {
                "target": reference_plan.target,
                "address": (
                    f"0x{reference_plan.address:08X}"
                    if reference_plan.address is not None
                    else None
                ),
                "bytes": "identical" if same else "different",
            }
        )


def load_manifests_for(plan: RenamePlan):
    if plan.manifests is None:
        plan.manifests = load_target_manifests(plan.root)
    return plan.manifests


MAX_FOREIGN_FILES = 60
MAX_FOREIGN_TARGETS = 12


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
        plan.foreign.setdefault(owner, []).append(source)
    if plan.address is not None:
        for owner in plan.foreign:
            rows = load_map(map_path(root, owner))
            if not any(symbol.address == plan.address for symbol in rows):
                plan.inserts[owner] = plan.address
    files = sum(len(paths) for paths in plan.foreign.values())
    if files > MAX_FOREIGN_FILES or len(plan.foreign) > MAX_FOREIGN_TARGETS:
        plan.refuse(
            "fan-out",
            f"{files} files in {len(plan.foreign)} other targets spell {old}; the "
            f"transaction holds at {MAX_FOREIGN_FILES} files and "
            f"{MAX_FOREIGN_TARGETS} targets",
        )


def plan_report(root: Path, target: str, old: str, new: str) -> dict:
    """Dry run: name every surface the rename would touch."""
    root = root.resolve()
    plan = _resolve(root, target, old, new)
    return _payload(root, plan, applied=False, proof=(), attempted=False)


def _foreign_proof(plan: RenamePlan) -> str | None:
    """Prove the targets whose units the rename rewrote still build and link.

    Compiling is not enough: a payload resolves a foreign symbol through a generated
    stub, which only follows the new spelling after a rebuild. Each foreign target is
    therefore re-checked, and any link finding fails the transaction.
    """
    from harness.build.compilation import compile_object
    from harness.io import repo_layout
    from harness.match._asm_resolve import object_path_for_source

    layout = repo_layout(plan.root)
    for _owner, paths in sorted(plan.foreign.items()):
        for relative in paths:
            path = plan.root / relative
            if not path.is_file() or path.suffix != ".c":
                continue
            try:
                output = object_path_for_source(layout, path)
            except (OSError, ValueError):
                continue
            try:
                result = compile_object(plan.root, path, output)
            except ValueError:
                # A grouped unit cannot be compiled on its own; its target check below
                # covers it, and the caller that owns the row keeps its own proof.
                continue
            except (OSError, RuntimeError) as error:
                return f"{relative}: {str(error)[:160]}"
            if result.returncode != 0:
                detail = (result.stderr or result.stdout or "").strip().splitlines()
                return f"{relative}: {detail[-1][:160] if detail else 'build failed'}"
    from harness.build.inventory import refresh
    from harness.domain.manifests import load_target_manifests

    from .inspection import check_report

    # The write changed maps, so the proof must read the generation that describes
    # the tree as it is now, not the one the plan was resolved against.
    generation = load_target_manifests(plan.root)
    plan.manifests = generation
    plan.manifest = generation.get(plan.target) or plan.manifest
    for owner in sorted(plan.foreign):
        # Relink against the current maps before the proof reads the result.
        try:
            if refresh(plan.root, owner)["link"] not in (None, 0):
                return f"{owner} does not link after a rebuild"
        except (OSError, RuntimeError, ValueError) as error:
            return f"{owner} could not be rebuilt: {str(error)[:160]}"
        report = check_report(plan.root, owner, (), manifests=generation)
        broken = [row for row in report["findings"] if row["code"] == "native"]
        if broken:
            return f"{owner} no longer links: {broken[0]['message'][:120]}"
    return None


def apply_report(root: Path, target: str, old: str, new: str) -> dict:
    """Apply the rename and prove its measurement did not change."""
    root = root.resolve()
    plan = _resolve(root, target, old, new)
    if not plan.passed:
        return _payload(root, plan, applied=False, proof=(), attempted=True)
    journal = _Journal(root)
    try:
        with acquire_writer(root):
            try:
                before = _measure(plan, plan.source)
                try:
                    _write(plan, old, new, journal)
                except (RuntimeError, ValueError, OSError) as error:
                    raise RuntimeError(f"write refused: {error}") from error
                broken = _foreign_proof(plan)
                if broken:
                    raise RuntimeError(f"foreign build failed: {broken}")
                # The manifest lists claimed unit paths, so the generation that
                # describes the renamed tree is the one the proof must read.
                plan.manifests = load_target_manifests(root)
                plan.manifest = plan.manifests.get(plan.target) or plan.manifest
                after = _measure(plan, _source_after(plan))
                if before != after:
                    raise RuntimeError("proof: the renamed source no longer measures the same")
            except (RuntimeError, ValueError, OSError) as error:
                journal.rollback()
                plan.refuse("measure", str(error).strip()[:200])
                return _payload(root, plan, applied=False, proof=(), attempted=True)
    except RuntimeError as error:
        plan.refuse("lease", str(error))
        return _payload(root, plan, applied=False, proof=(), attempted=True)
    _remember(root, plan)
    return _payload(root, plan, applied=True, proof=(before, after), attempted=True)


def _batch_rows(rows: list[dict]) -> tuple[list[tuple[str, str, str]], list[dict], list[dict]]:
    """Split batch rows into single renames and group renames, validating shape.

    Two rows of one target may not carry one new spelling: the second write would
    duplicate the map entry, so it is dropped and reported.
    """
    singles: list[tuple[str, str, str]] = []
    groups: list[dict] = []
    dropped: list[dict] = []
    taken: set[tuple[str, str]] = set()
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            raise ValueError(f"row {index}: expected an object")
        new = row.get("new")
        members = row.get("members")
        if members:
            if not isinstance(members, list) or not members:
                raise ValueError(f"row {index}: members must be a nonempty list")
            pairs = []
            for member in members:
                if not isinstance(member, dict) or "target" not in member or "old" not in member:
                    raise ValueError(f"row {index}: each member needs target and old")
                pairs.append((str(member["target"]), str(member["old"])))
            if not new:
                raise ValueError(f"row {index}: a group row needs new")
            groups.append({"new": str(new), "members": pairs})
            continue
        if not (row.get("target") and row.get("old") and new):
            raise ValueError(f"row {index}: needs target, old and new")
        key = (str(row["target"]), str(new).casefold())
        if key in taken:
            dropped.append(
                {
                    "target": key[0],
                    "old": str(row["old"]),
                    "new": str(new),
                    "code": "plan-duplicate",
                    "message": f"{new} is already claimed by an earlier row of this plan",
                }
            )
            continue
        taken.add(key)
        singles.append((str(row["target"]), str(row["old"]), str(new)))
    return singles, groups, dropped


def plan_batch(root: Path, rows: list[dict]) -> dict:
    """Dry run for a batch: every single row and every group, nothing written."""
    root = root.resolve()
    singles, groups, dropped = _batch_rows(rows)
    records = [
        _payload(
            root,
            _resolve(root, target, old, new),
            applied=False,
            proof=(),
            attempted=False,
        )
        for target, old, new in singles
    ]
    for group in groups:
        planned = plan_group(root, group["new"], group["members"])
        records.extend(
            {**member, "action": "rename-group"} for member in planned["members"]
        )
    return {
        "operation": "naming",
        "action": "batch",
        "applied": False,
        "passed": all(record["passed"] for record in records),
        "records": records,
        "refusals": dropped
        + [
            {"target": record["scope"]["target"], **refusal}
            for record in records
            for refusal in record["refusals"]
        ],
    }


def apply_rows(root: Path, rows: list[dict], *, jobs: int = 4) -> dict:
    """Apply a batch: single rows share one manifest and compile pass, groups stay atomic."""
    root = root.resolve()
    singles, groups, dropped = _batch_rows(rows)
    records = apply_batch(root, singles, jobs=jobs) if singles else []
    for group in groups:
        applied = apply_group(root, group["new"], group["members"], jobs=jobs)
        records.extend(
            {**member, "action": "rename-group"} for member in applied["members"]
        )
    return {
        "operation": "naming",
        "action": "batch",
        "applied": bool(records) and all(record["applied"] for record in records),
        "passed": bool(records) and all(record["passed"] for record in records),
        "records": records,
        "refusals": [
            {"target": record["scope"]["target"], **refusal}
            for record in records
            for refusal in record["refusals"]
        ],
    }


def _claim_owners(root: Path, manifests) -> dict[Path, set[str]]:
    """Every claimed source path with the targets that claim it."""
    owners: dict[Path, set[str]] = {}
    for target, manifest in manifests.items():
        try:
            paths = manifest_source_paths(root, manifest)
        except (OSError, ValueError):
            continue
        for path in paths:
            owners.setdefault(path, set()).add(target)
    return owners


def _retarget_twin(plan: RenamePlan, root: Path) -> None:
    """A twin takes a numbered filename when another target owns the plain one.

    Two units of one family cannot share a file, and the decoupled filename lets
    them share the symbol spelling instead.
    """
    if not plan.renames:
        return
    before, after = plan.renames[0]
    if not after.exists():
        return
    if after == plan.source:
        # Renaming a unit onto its own path is not a collision.
        return
    number = 2
    while True:
        candidate = after.with_name(f"{plan.new}{number}.c")
        if not candidate.exists() and not _claim_owners(
            root, load_manifests_for(plan)
        ).get(candidate):
            plan.renames[0] = (before, candidate)
            return
        number += 1


def _retarget_group_filenames(plans: list[RenamePlan]) -> None:
    """Give a twin its own filename when the family already uses the shared name.

    Two byte-identical units in one family share the spelling but cannot share one
    file, so the file takes the smallest free numbered form of that spelling while the
    symbol keeps the shared name.
    """
    taken = {
        plan.source
        for plan in plans
        if plan.source is not None and plan.old == plan.new
    }
    for plan in plans:
        if not plan.renames or plan.old == plan.new:
            continue
        before, after = plan.renames[0]
        if after.exists() or after in taken:
            number = 2
            while True:
                candidate = after.with_name(f"{plan.new}{number}.c")
                if not candidate.exists() and candidate not in taken:
                    after = candidate
                    break
                number += 1
            plan.renames[0] = (before, after)
        taken.add(after)


def _group_plans(root: Path, new: str, selections: list[tuple[str, str]]) -> list[RenamePlan]:
    """Resolve every member of one shared spelling with the group in view."""
    targets = frozenset(normalize_target_id(target).value for target, _old in selections)
    plans = [
        _resolve(root, target, old, new, group=targets, allow_noop=True)
        for target, old in selections
    ]
    _record_group_bytes(plans, root)
    _retarget_group_filenames(plans)
    for plan in plans:
        for other in plans:
            if other is plan or not other.passed or not plan.passed:
                continue
            if other.new.casefold() == plan.new.casefold():
                continue
    return plans


def plan_group(root: Path, new: str, selections: list[tuple[str, str]]) -> dict:
    """Dry run for one spelling shared by several targets."""
    root = root.resolve()
    plans = _group_plans(root, new, selections)
    return {
        "operation": "naming",
        "action": "rename-group",
        "new": new,
        "applied": False,
        "passed": all(plan.passed for plan in plans) and len(plans) > 1,
        "members": [
            _payload(root, plan, applied=False, proof=(), attempted=False)
            for plan in plans
        ],
        "refusals": [
            {"target": plan.target, **row} for plan in plans for row in plan.refusals
        ],
    }


def apply_group(root: Path, new: str, selections: list[tuple[str, str]], *, jobs: int = 4) -> dict:
    """Apply one spelling to every member, or to none of them.

    Each member keeps its own write and proof; the group is atomic because a
    shared spelling half-applied leaves the family inconsistent.
    """
    root = root.resolve()
    plans = _group_plans(root, new, selections)
    payload = {
        "operation": "naming",
        "action": "rename-group",
        "new": new,
        "applied": False,
        "passed": False,
        "members": [],
        "refusals": [
            {"target": plan.target, **row} for plan in plans for row in plan.refusals
        ],
    }
    if not all(plan.passed for plan in plans) or len(plans) < 2:
        payload["members"] = [
            _payload(root, plan, applied=False, proof=(), attempted=False)
            for plan in plans
        ]
        return payload

    compile_objects_parallel(
        root, [plan.source for plan in plans if plan.source is not None], jobs=jobs
    )
    written: list[tuple[RenamePlan, dict]] = []
    journal = _Journal(root)
    try:
        with acquire_writer(root):
            for plan in plans:
                if plan.old == plan.new:
                    continue
                try:
                    before = _measure(plan, plan.source)
                    _write(plan, plan.old, plan.new, journal)
                    broken = _foreign_proof(plan)
                    if broken:
                        raise RuntimeError(f"foreign build failed: {broken}")
                except (RuntimeError, ValueError, OSError) as error:
                    journal.rollback()
                    plan.refuse("write", str(error).strip()[:200])
                    payload["refusals"].append({"target": plan.target, **plan.refusals[-1]})
                    payload["members"] = [
                        _payload(root, item, applied=False, proof=(), attempted=True)
                        for item in plans
                    ]
                    return payload
                written.append((plan, before))

            generation = load_target_manifests(root)
            moved = [_source_after(plan) for plan, _proof in written]
            compile_objects_parallel(root, [path for path in moved if path], jobs=jobs)

            proofs: list[tuple[RenamePlan, dict, dict]] = []
            failure = None
            for plan, before in written:
                plan.manifests = generation
                plan.manifest = generation.get(plan.target) or plan.manifest
                try:
                    after = _measure(plan, _source_after(plan))
                except (RuntimeError, ValueError, OSError) as error:
                    failure = (plan, f"measure: {str(error).strip()[:200]}")
                    break
                if before != after:
                    failure = (plan, "proof: the renamed row no longer measures the same")
                    break
                proofs.append((plan, before, after))
            if failure is not None:
                journal.rollback()
                failing, message = failure
                failing.refuse(message.split(":")[0], message.split(": ", 1)[-1])
                payload["refusals"].append({"target": failing.target, **failing.refusals[-1]})
                payload["members"] = [
                    _payload(root, item, applied=False, proof=(), attempted=True)
                    for item in plans
                ]
                return payload

            for plan, before, after in proofs:
                _remember(root, plan)
                payload["members"].append(
                    _payload(root, plan, applied=True, proof=(before, after), attempted=True)
                )
            payload["applied"] = True
            payload["passed"] = True
    except RuntimeError as error:
        journal.rollback()
        payload["refusals"].append({"target": "", "code": "lease", "message": str(error)})
        payload["members"] = [
            _payload(root, item, applied=False, proof=(), attempted=True) for item in plans
        ]
    return payload


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
    journals: dict[int, _Journal] = {}
    try:
        with acquire_writer(root):
            for index, plan in enumerate(ready):
                # An earlier row may have renamed a unit this row references, so the
                # consumer list is refreshed against the tree as it is now.
                plan.files = []
                plan.renames = []
                try:
                    _collect_files(plan, root, plan.manifest)
                    if plan.passed:
                        _retarget_twin(plan, root)
                except (OSError, ValueError):
                    pass
                try:
                    row_journal = _Journal(root)
                    before = _measure(plan, plan.source)
                    _write(plan, plan.old, plan.new, row_journal)
                    broken = _foreign_proof(plan)
                    if broken:
                        raise RuntimeError(f"foreign build failed: {broken}")
                except (FileNotFoundError, ValueError) as error:
                    row_journal.rollback()
                    # A batch's plan can lose its unit to an earlier row's rename;
                    # re-resolve this row against the tree and try it once more.
                    fresh = _resolve(root, plan.target, plan.old, plan.new)
                    if fresh.passed:
                        ready[index] = fresh
                        try:
                            before = _measure(fresh, fresh.source)
                            row_journal = _Journal(root)
                            _write(fresh, fresh.old, fresh.new, row_journal)
                            broken = _foreign_proof(fresh)
                            if broken:
                                raise RuntimeError(f"foreign build failed: {broken}")
                        except (RuntimeError, ValueError, OSError) as again:
                            row_journal.rollback()
                            fresh.refuse("write", str(again).strip()[:200])
                            payloads.append(
                                _payload(
                                    root, fresh, applied=False, proof=(), attempted=True
                                )
                            )
                            continue
                        journals[id(fresh)] = row_journal
                        results.append((fresh, before))
                        continue
                    plan.refuse("write", str(error).strip()[:200])
                    payloads.append(
                        _payload(root, plan, applied=False, proof=(), attempted=True)
                    )
                    continue
                except (RuntimeError, OSError) as error:
                    row_journal.rollback()
                    plan.refuse("write", str(error).strip()[:200])
                    payloads.append(
                        _payload(root, plan, applied=False, proof=(), attempted=True)
                    )
                    continue
                journals[id(plan)] = row_journal
                results.append((plan, before))

            if results:
                try:
                    generation = load_target_manifests(root)
                except (ValueError, OSError) as error:
                    for plan, _before in results:
                        journals[id(plan)].rollback()
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
                    journals[id(plan)].rollback()
                    plan.refuse("measure", str(error).strip()[:200])
                    payloads.append(
                        _payload(root, plan, applied=False, proof=(before,), attempted=True)
                    )
                    continue
                if before != after:
                    journals[id(plan)].rollback()
                    plan.refuse("proof", "the renamed source no longer measures the same")
                    payloads.append(
                        _payload(
                            root, plan, applied=False, proof=(before, after), attempted=True
                        )
                    )
                    continue
                _remember(root, plan)
                payloads.append(
                    _payload(root, plan, applied=True, proof=(before, after), attempted=True)
                )
    except RuntimeError as error:
        for plan in ready:
            if not any(row is plan for row, _ in results):
                continue
            journals[id(plan)].rollback()
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


class _Journal:
    """Record every byte a transaction replaces, so any failure restores the tree.

    A rename touches the owning target, the foreign units that spell the same name
    and the files it moves. Reversing those writes one at a time is how a partial
    rename survived a failed group, so each path is captured before it is written and
    the whole journal is replayed on failure.
    """

    def __init__(self, root: Path) -> None:
        self.root = root
        self.entries: dict[Path, bytes | None] = {}

    def capture(self, path: Path) -> None:
        if path in self.entries:
            return
        self.entries[path] = path.read_bytes() if path.is_file() else None

    def write_text(self, path: Path, text: str) -> None:
        self.capture(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def move(self, before: Path, after: Path) -> None:
        self.capture(before)
        self.capture(after)
        before.rename(after)

    def rollback(self) -> list[str]:
        restored = []
        for path, content in reversed(list(self.entries.items())):
            try:
                if content is None:
                    if path.exists():
                        path.unlink()
                elif path.read_bytes() != content:
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes(content)
                restored.append(path.relative_to(self.root).as_posix())
            except OSError:
                restored.append(f"{path}: unconfirmed")
        _CENSUS.pop(self.root, None)
        _MEASURED.clear()
        return restored


def _write(
    plan: RenamePlan,
    source_name: str,
    target_name: str,
    journal: _Journal | None = None,
) -> None:
    """Rewrite the owned surface and the foreign spellings, then move the unit.

    With a journal every write is captured first, so a caller can restore the tree
    exactly; without one the caller is responsible for reversing the write.
    """
    _CENSUS.pop(plan.root, None)
    _MEASURED.clear()
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
        after = plan.renames[0][1]
        before_key = _unit_key(plan.root, plan.source)
        after_key = _unit_key(plan.root, after)
    else:
        before_key, after_key = "", ""
    for path in plan.files:
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8")
        # Paths first: a claim path carries the identifier, so rewriting the name
        # before the path would make the path rule match nothing.
        rewritten = text
        for old_path, new_path in paths:
            rewritten = (
                rewritten.replace(old_path, new_path)
                if forward
                else rewritten.replace(new_path, old_path)
            )
        rewritten = pattern.sub(target_name, rewritten)
        if before_key and path == plan.root / CONFIGURATION_PATH:
            rewritten = rewritten.replace(before_key, after_key)
        if rewritten != text:
            if journal is None:
                path.write_text(rewritten, encoding="utf-8")
            else:
                journal.write_text(path, rewritten)
    for _owner, paths in sorted(plan.foreign.items()):
        for relative in paths:
            path = plan.root / relative
            if not path.is_file():
                continue
            text = path.read_text(encoding="utf-8")
            rewritten = text.replace(source_name, target_name)
            if rewritten == text:
                continue
            # A unit that aliases the spelling for a shared template keeps its
            # `#define` intact; rewriting the macro name would make it self-referential.
            alias = re.compile(rf"^(\s*#\s*define\s+){re.escape(source_name)}\b")
            lines = text.splitlines(keepends=True)
            if len(lines) != len(rewritten.splitlines(keepends=True)):
                lines = None
            if lines is not None:
                rewritten = "".join(
                    line if alias.match(line) else line.replace(source_name, target_name)
                    for line in lines
                )
            if plan.address is not None:
                # A map row is tracked through an @source-tagged definition, so a
                # declaration the rewrite touched gains the tag when it lacks one.
                declaration = re.compile(
                    rf"^(\s*extern\b[^;\n]*\b{re.escape(target_name)}\b[^;\n]*;)"
                    r"(?![ \t]*/\*)",
                    re.M,
                )
                rewritten = declaration.sub(
                    lambda m: f"{m.group(1)} /* @source 0x{plan.address:08X} @kind data */",
                    rewritten,
                )
            if journal is None:
                path.write_text(rewritten, encoding="utf-8")
            else:
                journal.write_text(path, rewritten)
    # A foreign target spells the name but may not map the address; the link resolves
    # the spelling in that target's own map, so the transaction adds the row.
    for owner, address in sorted(plan.inserts.items()):
        path = map_path(plan.root, owner)
        if not forward or journal is None or not path.is_file():
            continue
        rows = [
            line for line in path.read_text(encoding="utf-8").splitlines() if line.strip()
        ]
        if any(line.startswith(f"{plan.new} =") for line in rows):
            continue
        rows.append(f"{plan.new} = 0x{address:08X};")
        rows.sort(key=lambda line: int(line.split("=")[1].strip().rstrip(";"), 16))
        journal.write_text(path, "\n".join(rows) + "\n")
    if forward:
        seen = set()
        for before, after in plan.renames:
            if (before, after) in seen:
                continue
            seen.add((before, after))
            if journal is None:
                before.rename(after)
            else:
                journal.move(before, after)


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
        "shared_with": list(plan.shared_with),
        "passed": plan.passed and (applied or not attempted),
    }
