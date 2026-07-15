from __future__ import annotations

import json
import os
import re
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any


EVIDENCE_LADDER = [
    "draft",
    "compiles",
    "same-size",
    "improving",
    "high-confidence",
    "instruction-exact",
    "byte-exact",
]

MODULE_MILESTONES = [
    "mapped",
    "segmented",
    "liftable",
    "behavior-understood",
    "compiled",
    "functions-exact",
    "module-complete",
]


@dataclass
class MissionState:
    mission_id: str
    target_id: str
    address: int | None
    goal: str
    strategy: str
    status: str
    attempts: int
    max_attempts: int = 3
    budget_functions: int = 1
    budget_time_seconds: int = 1800
    budget_depth: int = 1
    created_at: float = field(default_factory=time.time)
    updated_at: float = field(default_factory=time.time)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    @classmethod
    def from_dict(cls, payload: dict[str, Any]) -> MissionState:
        return cls(**payload)


def _make_mission_id(target_id: str, address: int | None) -> str:
    if address is None:
        return f"{target_id}_none"
    return f"{target_id}_{address:#010x}"


def _reverse_dir(root: Path, target_id: str) -> Path:
    return root / "out" / "reverse" / target_id


def _function_dir(root: Path, target_id: str, address: int | None) -> Path:
    if address is None:
        return _reverse_dir(root, target_id) / "functions" / "unknown"
    return _reverse_dir(root, target_id) / "functions" / f"func_{address:08x}"


def plan_mission(
    root: Path,
    target_id: str,
    address: int | None,
    *,
    goal: str | None = None,
    strategy: str = "balanced",
    max_attempts: int = 3,
    budget_functions: int = 1,
    budget_time_seconds: int = 1800,
    budget_depth: int = 1,
    **opts: Any,
) -> MissionState:
    mission_id = _make_mission_id(target_id, address)
    inferred_goal = goal or (
        infer_goal(root, target_id, address) if address is not None else "lift"
    )
    now = time.time()
    state: dict[str, Any] = {
        "mission_id": mission_id,
        "target_id": target_id,
        "address": address,
        "goal": inferred_goal,
        "strategy": strategy,
        "status": "pending",
        "attempts": 0,
        "max_attempts": max_attempts,
        "budget_functions": budget_functions,
        "budget_time_seconds": budget_time_seconds,
        "budget_depth": budget_depth,
        "created_at": now,
        "updated_at": now,
    }
    for key, value in opts.items():
        if key in state:
            state[key] = value
    return MissionState(**state)


def load_mission(root: Path, mission_id: str) -> MissionState | None:
    if mission_id.endswith("_none"):
        target_id = mission_id[:-5]
        address = None
    else:
        parts = mission_id.rsplit("_0x", 1)
        if len(parts) == 2:
            target_id, addr_hex = parts
            try:
                address = int(addr_hex, 16)
            except ValueError:
                return _scan_for_mission(root, mission_id)
        else:
            return _scan_for_mission(root, mission_id)

    path = _function_dir(root, target_id, address) / "mission.json"
    if not path.exists():
        return None
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
        return MissionState.from_dict(payload)
    except (json.JSONDecodeError, KeyError, TypeError):
        return None


def _scan_for_mission(root: Path, mission_id: str) -> MissionState | None:
    reverse_root = root / "out" / "reverse"
    if not reverse_root.exists():
        return None
    for target_dir in reverse_root.iterdir():
        funcs_dir = target_dir / "functions"
        if not funcs_dir.exists():
            continue
        for func_dir in funcs_dir.iterdir():
            mission_path = func_dir / "mission.json"
            if not mission_path.exists():
                continue
            try:
                payload = json.loads(mission_path.read_text(encoding="utf-8"))
                if payload.get("mission_id") == mission_id:
                    return MissionState.from_dict(payload)
            except (json.JSONDecodeError, KeyError, TypeError):
                continue
    return None


def save_mission(root: Path, mission: MissionState) -> Path:
    func_dir = _function_dir(root, mission.target_id, mission.address)
    path = func_dir / "mission.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_suffix(".json.tmp")
    temp.write_text(json.dumps(mission.to_dict(), indent=2), encoding="utf-8")
    os.replace(temp, path)
    return path


def preview_mission(root: Path, mission: MissionState) -> dict[str, Any]:
    inferred = (
        infer_goal(root, mission.target_id, mission.address)
        if mission.address is not None
        else "lift"
    )
    result: dict[str, Any] = {
        "target": mission.target_id,
        "address": mission.address,
        "inferred_goal": inferred,
        "strategy": mission.strategy,
        "budget": {
            "functions": mission.budget_functions,
            "time_seconds": mission.budget_time_seconds,
            "depth": mission.budget_depth,
        },
        "exclusions": [],
        "alternatives": [],
    }
    if mission.address is None:
        next_func = select_next_function(root, mission.target_id)
        if next_func is not None:
            address, goal = next_func
            result["suggested_function"] = {
                "address": address,
                "goal": goal,
            }
            result["inferred_goal"] = goal
            result["address"] = address
    return result


def _source_path(root: Path, target_id: str, address: int) -> Path:
    """Return the expected source file path for a function."""
    source_dir = _source_dir(root, target_id)
    return source_dir / f"func_{address:08x}.c"


def _match_summary_path(root: Path, target_id: str, address: int) -> Path:
    """Return the expected asm-differ summary path for a function."""
    manifest = _target_manifest(root, target_id)
    if manifest is not None:
        source_dir = Path(manifest.source_dir).as_posix()
        target_slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", source_dir)
        return (
            root
            / "out"
            / "matching"
            / target_slug
            / f"func_{address:08x}"
            / "asm-differ"
            / "summary.json"
        )
    direct = (
        root
        / "out"
        / "matching"
        / target_id
        / f"func_{address:08x}"
        / "asm-differ"
        / "summary.json"
    )
    if direct.is_file():
        return direct
    if target_id.startswith("emi/"):
        slug = target_id.removeprefix("emi/")
        emi = (
            root
            / "out"
            / "matching"
            / "emi"
            / slug
            / f"func_{address:08x}"
            / "asm-differ"
            / "summary.json"
        )
        if emi.is_file():
            return emi
    return direct


def infer_goal(root: Path, target_id: str, address: int) -> str:
    source = _source_path(root, target_id, address)
    if not source.is_file():
        return "lift"

    summary = _match_summary_path(root, target_id, address)
    if not summary.is_file():
        return "improve"

    try:
        data = json.loads(summary.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, KeyError, TypeError):
        return "improve"

    match_pct = data.get("instruction_count", {}).get("match_percent")
    if match_pct == 100:
        bytes_pct = data.get("bytes", {}).get("match_percent")
        if bytes_pct == 100:
            return "select the next eligible function"

    return "match"


def _splat_config_path(root: Path, target_id: str) -> Path:
    """Return the Splat config path for a target, preferring EMI conventions."""
    manifest = _target_manifest(root, target_id)
    if manifest is not None:
        return root / manifest.splat
    if target_id.startswith("emi/"):
        slug = target_id.removeprefix("emi/")
        emi = root / "config" / "splat" / "emi" / f"{slug}.yaml"
        if emi.is_file():
            return emi
    direct = root / "config" / "splat" / f"{target_id}.yaml"
    return direct


def _source_dir(root: Path, target_id: str) -> Path:
    """Return the source directory for a target, preferring EMI conventions."""
    manifest = _target_manifest(root, target_id)
    if manifest is not None:
        return root / manifest.source_dir
    if target_id.startswith("emi/"):
        slug = target_id.removeprefix("emi/")
        emi = root / "src" / "emi" / slug
        if emi.is_dir():
            return emi
    return root / "src" / target_id


def _matching_dir(root: Path, target_id: str) -> Path:
    """Return the matching output directory for a target."""
    manifest = _target_manifest(root, target_id)
    if manifest is not None:
        source_dir = Path(manifest.source_dir).as_posix()
        target_slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", source_dir)
        return root / "out" / "matching" / target_slug
    if target_id.startswith("emi/"):
        slug = target_id.removeprefix("emi/")
        emi = root / "out" / "matching" / "emi" / slug
        if emi.is_dir():
            return emi
    return root / "out" / "matching" / target_id


def _load_address_for_target(root: Path, target_id: str) -> int:
    """Try to resolve the target load address from the EMI catalog."""
    manifest = _target_manifest(root, target_id)
    if manifest is not None:
        return manifest.load_address
    catalog_path = root / "out" / "catalog" / "emi.json"
    if not catalog_path.is_file():
        return 0
    try:
        catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        for entry in catalog.get("entries", []):
            slug = entry["archive_id"].replace("/", "_").lower() + f"_{entry['slot']:02d}"
            if target_id == f"emi/{slug}" or target_id == f"emi_{slug}":
                return int(entry.get("load_address", 0))
            archive = entry["archive_id"].replace("/", "_").lower()
            if target_id == f"emi_{archive}_{entry['slot']:02d}":
                return int(entry.get("load_address", 0))
    except (json.JSONDecodeError, KeyError, TypeError, ValueError):
        pass
    return 0


def _target_manifest(root: Path, target_id: str):
    from .domain import load_target_manifests, normalize_target_id

    try:
        normalized = normalize_target_id(target_id).value
    except ValueError:
        return None
    return load_target_manifests(root).get(normalized)


def score_candidates(root: Path, target_id: str) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []

    config = _splat_config_path(root, target_id)
    load_address = _load_address_for_target(root, target_id)

    reviewed: list[int] = []
    if config.is_file():
        from .binaries import SPLAT_FUNCTION_SUBSEGMENT_RE

        for line in config.read_text(encoding="utf-8").splitlines():
            match = SPLAT_FUNCTION_SUBSEGMENT_RE.match(line)
            if match is not None:
                reviewed.append(load_address + int(match.group("offset"), 0))
        reviewed = sorted(set(reviewed))

    if not reviewed:
        return candidates

    source_dir = _source_dir(root, target_id)
    match_dir = _matching_dir(root, target_id)

    for address in reviewed:
        source = source_dir / f"func_{address:08x}.c"
        summary = match_dir / f"func_{address:08x}" / "asm-differ" / "summary.json"

        if not source.is_file():
            score = 100
            state = "not_lifted"
            reason = "No source file; needs lifting"
        elif not summary.is_file():
            score = 50
            state = "lifted_not_matching"
            reason = "Source exists but no match data yet"
        else:
            try:
                data = json.loads(summary.read_text(encoding="utf-8"))
                match_pct = data.get("instruction_count", {}).get("match_percent", 0)
                bytes_pct = data.get("bytes", {}).get("match_percent", 0)
                if match_pct == 100 and bytes_pct == 100:
                    score = 0
                    state = "byte_exact"
                    reason = "Fully matched"
                else:
                    score = 25
                    state = "matching_not_exact"
                    reason = f"Match at {match_pct}% instructions, {bytes_pct}% bytes"
            except (json.JSONDecodeError, KeyError, TypeError):
                score = 50
                state = "lifted_not_matching"
                reason = "Source exists but match data unreadable"

        if score > 0:
            candidates.append(
                {
                    "address": address,
                    "name": f"func_{address:08x}",
                    "score": score,
                    "state": state,
                    "reason": reason,
                }
            )

    return sorted(candidates, key=lambda c: (-c["score"], c["address"]))


def _get_code_ranges(splat_path: Path, load_address: int) -> list[tuple[int, int]]:
    """Return list of (start_addr, end_addr) for code segments from splat config.

    Filters out false-positive function detections from data areas.
    """

    import re

    if not splat_path.is_file():
        return []

    text = splat_path.read_text(encoding="utf-8")
    lines = text.splitlines()

    # Find all code segments and their vram addresses.
    code_segments: list[int] = []
    for i, line in enumerate(lines):
        if "type: code" not in line:
            continue
        for j in range(max(0, i - 5), i + 1):
            m = re.search(r"vram:\s+0x([0-9a-fA-F]+)", lines[j])
            if m:
                code_segments.append(int(m.group(1), 16))
                break

    # Find all segment boundaries.
    boundaries: list[int] = []
    for line in lines:
        m = re.match(r"\s+-\s+\[0x([0-9a-fA-F]+)", line)
        if m:
            boundaries.append(load_address + int(m.group(1), 16))

    # Build code ranges: each code segment ends at the next boundary or
    # next code segment start.
    ranges: list[tuple[int, int]] = []
    for i, vram in enumerate(code_segments):
        end = None
        for b in boundaries:
            if b > vram:
                end = b
                break
        if i + 1 < len(code_segments):
            next_vram = code_segments[i + 1]
            if end is None or next_vram < end:
                end = next_vram
        if end is not None:
            ranges.append((vram, end))

    return ranges


def score_candidates_all(
    root: Path,
    *,
    strategy: str = "leaf",
    limit: int = 10,
) -> list[dict[str, Any]]:
    """Scan all promoted targets with rizin and rank leaf/root/hot candidates.

    Scoring metric (higher = more valuable to decompile):

        score = size                                     # code surface
              + (size if is_leaf else 0)                 # leaf bonus: self-contained
              + (300 if not lifted else 0)               # prefer new work
              + (25 if reviewed_boundary else 0)         # confirmed boundary
              + calls_in * 50                            # foundational: called by many
              + (100 if target_has_context else 0)       # target infra bonus
              - (max(0, 200 - size) if size < 32 else 0) # stub penalty

    A leaf function of 500 bytes, not yet lifted, called by 5 others, in a
    target with good infrastructure scores:
        500 + 500 + 300 + 0 + 250 + 100 = 1675
    """

    from .analyzer import build_snapshot, find_best_engine
    from .domain import load_target_manifests

    manifests = load_target_manifests(root)
    engine = find_best_engine()
    all_candidates: list[dict[str, Any]] = []

    for target_id, manifest in sorted(manifests.items()):
        binary_path = root / manifest.binary
        if not binary_path.is_file():
            continue

        # Get code ranges from splat config to filter out false positives.
        code_ranges = _get_code_ranges(root / manifest.splat, manifest.load_address)

        reviewed: set[int] = set()
        splat_path = root / manifest.splat
        if splat_path.is_file():
            from .binaries import SPLAT_FUNCTION_SUBSEGMENT_RE

            for line in splat_path.read_text(encoding="utf-8").splitlines():
                match = SPLAT_FUNCTION_SUBSEGMENT_RE.match(line)
                if match is not None:
                    reviewed.add(
                        manifest.load_address + int(match.group("offset"), 0)
                    )

        try:
            snapshot = build_snapshot(
                engine,
                binary_path,
                manifest.load_address,
                target_id,
                reviewed_addresses=reviewed,
                source_dir=root / manifest.source_dir,
                timeout=60,
            )
        except Exception:
            continue

        # Count incoming calls per function (foundational = called by many).
        callers_out: dict[str, int] = {}
        calls_in: dict[str, int] = {}
        for call in snapshot.calls:
            callers_out[call.caller] = callers_out.get(call.caller, 0) + 1
            calls_in[call.callee] = calls_in.get(call.callee, 0) + 1

        # Target infrastructure: count lifted functions in this target.
        lifted_count = sum(1 for f in snapshot.functions if f.source is not None)
        target_has_context = lifted_count >= 10

        for func in snapshot.functions:
            # Filter out false positives: only include functions within code ranges.
            if code_ranges:
                in_code = any(
                    start <= func.address < end for start, end in code_ranges
                )
                if not in_code:
                    continue

            is_leaf = func.id not in callers_out
            n_calls_out = callers_out.get(func.id, 0)
            n_calls_in = calls_in.get(func.id, 0)

            if strategy == "leaf" and not is_leaf:
                continue
            if strategy == "root" and n_calls_out == 0:
                continue

            source_exists = func.source is not None
            state = "lifted" if source_exists else "not_lifted"
            if func.is_reviewed:
                state = "reviewed" if not source_exists else "lifted+reviewed"

            size = func.analyzer_size

            # --- Scoring ---
            score = size

            # Leaf bonus: self-contained, no call dependencies.
            if is_leaf:
                score += size

            # Prefer unlifted functions (new work).
            if not source_exists:
                score += 300

            # Reviewed boundary bonus (confirmed function range).
            if func.is_reviewed:
                score += 25

            # Foundational bonus: functions called by many others.
            score += n_calls_in * 50

            # Target infrastructure bonus.
            if target_has_context:
                score += 100

            # Stub penalty: tiny functions are likely alignment/stubs.
            if size < 32:
                score -= 200 - size

            all_candidates.append(
                {
                    "target_id": target_id,
                    "address": func.address,
                    "name": func.analyzer_name,
                    "size": size,
                    "calls_out": n_calls_out,
                    "calls_in": n_calls_in,
                    "is_leaf": is_leaf,
                    "is_reviewed": func.is_reviewed,
                    "is_lifted": source_exists,
                    "score": score,
                    "state": state,
                    "source": func.source,
                }
            )

    all_candidates.sort(key=lambda c: -c["score"])
    return all_candidates[:limit]


def select_next_function(root: Path, target_id: str) -> tuple[int, str] | None:
    candidates = score_candidates(root, target_id)
    if not candidates:
        return None
    best = candidates[0]
    return (best["address"], infer_goal(root, target_id, best["address"]))
